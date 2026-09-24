#include "fastfetch.h"
#include "common/networking.h"
#include "common/time.h"
#include "common/library.h"
#include "common/strutil.h"
#include "common/mallocHelper.h"
#include "common/debug.h"

#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h> // For FreeBSD
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>

// Upper bound of a single HTTP response, guarding against excessive memory allocation
#define FF_NETWORKING_MAX_RESPONSE_SIZE (1024u * 1024u)

static const char* tryNonThreadingFastPath(FFNetworkingState* state) {
#if defined(TCP_FASTOPEN) || __APPLE__

    if (!state->tfo) {
    #if __linux__ || __GNU__
        // Linux doesn't support sendto() on unconnected sockets
        FF_DEBUG("TCP Fast Open disabled, skipping");
        return "TCP Fast Open disabled";
    #endif
    } else {
        FF_DEBUG("Attempting to use TCP Fast Open to connect");

    #ifndef __APPLE__ // On macOS, TCP_FASTOPEN doesn't seem to be needed
        // Set TCP Fast Open
        int flag = 1;
        if (setsockopt(state->sockfd, IPPROTO_TCP,
        #ifdef __APPLE__
                // https://github.com/rust-lang/libc/pull/3135
                0x218 // TCP_FASTOPEN_FORCE_ENABLE
        #else
                TCP_FASTOPEN
        #endif
                ,
                &flag,
                sizeof(flag)) != 0) {
            FF_DEBUG("Failed to set TCP_FASTOPEN option: %s", strerror(errno));
            return "setsockopt(TCP_FASTOPEN) failed";
        } else {
        #if __linux__ || __GNU__
            FF_DEBUG("Successfully set TCP_FASTOPEN option, queue length: %d", flag);
        #elif defined(__APPLE__)
            FF_DEBUG("Successfully set TCP_FASTOPEN_FORCE_ENABLE option");
        #else
            FF_DEBUG("Successfully set TCP_FASTOPEN option");
        #endif
        }
    #endif
    }

    #ifndef __APPLE__
    FF_DEBUG("Using sendto() "
        #ifdef MSG_FASTOPEN
            "+ MSG_FASTOPEN "
        #endif
        #ifdef MSG_NOSIGNAL
            "+ MSG_NOSIGNAL "
        #endif
        "to send %u bytes of data", state->command.length);
    ssize_t sent = sendto(state->sockfd,
        state->command.chars,
        state->command.length,
        #ifdef MSG_FASTOPEN
        MSG_FASTOPEN |
        #endif
        #ifdef MSG_NOSIGNAL
            MSG_NOSIGNAL |
        #endif
            0,
        state->addr->ai_addr,
        state->addr->ai_addrlen);
    #else
    if (fcntl(state->sockfd, F_SETFL, O_NONBLOCK) == -1) {
        FF_DEBUG("fcntl(F_SETFL) failed: %s", strerror(errno));
        return "fcntl(F_SETFL) failed";
    }
    FF_DEBUG("Using connectx() to send %u bytes of data", state->command.length);
    // Use connectx to establish connection and send data in one call
    size_t sent;
    if (connectx(state->sockfd,
            &(sa_endpoints_t) {
                .sae_dstaddr = state->addr->ai_addr,
                .sae_dstaddrlen = state->addr->ai_addrlen,
            },
            SAE_ASSOCID_ANY,
            state->tfo ? CONNECT_DATA_IDEMPOTENT : 0,
            &(struct iovec) {
                .iov_base = state->command.chars,
                .iov_len = state->command.length,
            },
            1,
            &sent,
            nullptr) != 0) {
        sent = 0;
    }
    if (fcntl(state->sockfd, F_SETFL, 0) == -1) {
        FF_DEBUG("fcntl(F_SETFL) failed: %s", strerror(errno));
        return "fcntl(F_SETFL) failed";
    }
    #endif
    if (sent > 0 || (errno == EAGAIN || errno == EWOULDBLOCK
    #ifdef __APPLE__
                        // On macOS EINPROGRESS means the connection cannot be completed immediately
                        // On Linux, it means the TFO cookie is not available locally
                        || errno == EINPROGRESS
    #endif
                        )) {
        FF_DEBUG(
    #ifdef __APPLE__
            "connectx()"
    #else
            "sendto()"
    #endif
            " %s (sent=%zd, %s)",
            errno == 0 ? "succeeded" : "was in progress",
            sent,
            strerror(errno));
        freeaddrinfo(state->addr);
        state->addr = nullptr;
        ffStrbufDestroy(&state->command);
        return nullptr;
    }

    FF_DEBUG(
    #ifdef __APPLE__
        "connectx()"
    #else
        "sendto()"
    #endif
        " failed: %s",
        strerror(errno));
    #ifdef __APPLE__
    return "connectx() failed";
    #else
    return "sendto() failed";
    #endif
#else
    FF_UNUSED(state);
    return "TFO support is not available";
#endif
}

// Traditional connect and send function
static const char* connectAndSend(FFNetworkingState* state) {
    const char* ret = nullptr;
    FF_DEBUG("Using traditional connection method to connect");

    FF_DEBUG("Attempting connect() to server...");
    if (connect(state->sockfd, state->addr->ai_addr, state->addr->ai_addrlen) == -1) {
        FF_DEBUG("connect() failed: %s", strerror(errno));
        ret = "connect() failed";
        goto error;
    }
    FF_DEBUG("connect() succeeded");

    FF_DEBUG("Attempting to send %u bytes of data...", state->command.length);
    if (send(state->sockfd, state->command.chars, state->command.length, 0) < 0) {
        FF_DEBUG("send() failed: %s", strerror(errno));
        ret = "send() failed";
        goto error;
    }
    FF_DEBUG("Data sent successfully");

    goto exit;

error:
    FF_DEBUG("Error occurred, closing socket");
    close(state->sockfd);
    state->sockfd = -1;

exit:
    FF_DEBUG("Releasing address info and other resources");
    freeaddrinfo(state->addr);
    state->addr = nullptr;
    ffStrbufDestroy(&state->command);

    return ret;
}

FF_THREAD_ENTRY_DECL_WRAPPER(connectAndSend, FFNetworkingState*);

// Parallel DNS resolution and socket creation
static const char* initNetworkingState(FFNetworkingState* state, const char* host, uint16_t port, const char* path, const char* headers) {
    FF_DEBUG("Initializing network connection state: host=%s, path=%s", host, path);

    // Initialize command and host information
    ffStrbufInitA(&state->command, 128);
    ffStrbufAppendS(&state->command, "GET ");
    ffStrbufAppendS(&state->command, path);
    ffStrbufAppendS(&state->command, " HTTP/1.0\r\nHost: ");
    if (strchr(host, ':') != nullptr) {
        // An IPv6 literal has to be bracketed in the Host header (RFC 9110 7.2), while
        // getaddrinfo() wants it bare
        ffStrbufAppendC(&state->command, '[');
        ffStrbufAppendS(&state->command, host);
        ffStrbufAppendC(&state->command, ']');
    } else {
        ffStrbufAppendS(&state->command, host);
    }
    // The Host header carries the port whenever it is not the default one (RFC 9110 7.2)
    if (port != 80) {
        ffStrbufAppendF(&state->command, ":%u", port);
    }
    ffStrbufAppendS(&state->command, "\r\nConnection: close\r\n"); // Explicitly tell the server we don't need to keep the connection

    // If compression needs to be enabled
    if (state->compression) {
        FF_DEBUG("Enabling HTTP content compression");
        ffStrbufAppendS(&state->command, "Accept-Encoding: gzip\r\n");
    }

    ffStrbufAppendS(&state->command, headers);
    ffStrbufAppendS(&state->command, "\r\n");

#ifdef FF_HAVE_THREADS
    state->thread = 0;
    FF_DEBUG("Thread ID initialized to 0");
#endif

    const char* ret = nullptr;

    struct addrinfo hints = {
        .ai_family = state->ipv6 ? AF_INET6 : AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_NUMERICSERV
    };

    char portA[6];
    snprintf(portA, sizeof(portA), "%u", port);

    FF_DEBUG("Resolving address: %s:%u (%s)", host, port, state->ipv6 ? "IPv6" : "IPv4");
    // Use AI_NUMERICSERV flag to indicate the service is a numeric port, reducing parsing time

    int gaiRes = getaddrinfo(host, portA, &hints, &state->addr);
    if (gaiRes != 0) {
        FF_DEBUG("getaddrinfo() failed: %s (res=%d)", gai_strerror(gaiRes), gaiRes);
        ret = "getaddrinfo() failed";
        goto error;
    }
    FF_DEBUG("Address resolution successful");

    FF_DEBUG("Creating socket");
    state->sockfd = socket(state->addr->ai_family, state->addr->ai_socktype, state->addr->ai_protocol);
    if (state->sockfd == -1) {
        FF_DEBUG("socket() failed: %s", strerror(errno));
        ret = "socket() failed";
        goto error;
    }
    FF_DEBUG("Socket creation successful: fd=%d", state->sockfd);

    int flag = 1;
#ifdef TCP_NODELAY
    // Disable Nagle's algorithm to reduce small packet transmission delay
    if (setsockopt(state->sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) != 0) {
        FF_DEBUG("Failed to set TCP_NODELAY: %s", strerror(errno));
    } else {
        FF_DEBUG("Successfully disabled Nagle's algorithm");
    }
#endif

#ifdef TCP_QUICKACK
    // Set TCP_QUICKACK option to avoid delayed acknowledgments
    if (setsockopt(state->sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) != 0) {
        FF_DEBUG("Failed to set TCP_QUICKACK: %s", strerror(errno));
    } else {
        FF_DEBUG("Successfully enabled TCP quick acknowledgment");
    }
#endif

#ifdef SO_NOSIGPIPE
    // Prevent SIGPIPE when the server closes the connection during write
    if (setsockopt(state->sockfd, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(flag)) != 0) {
        FF_DEBUG("Failed to set SO_NOSIGPIPE: %s", strerror(errno));
    } else {
        FF_DEBUG("Successfully set SO_NOSIGPIPE");
    }
#endif

    if (state->timeout > 0) {
        FF_DEBUG("Setting connection timeout: %u ms", state->timeout);
        [[maybe_unused]] uint32_t sec = state->timeout / 1000;
        if (sec == 0) {
            sec = 1;
        }

#ifdef TCP_CONNECTIONTIMEOUT
        FF_DEBUG("Using TCP_CONNECTIONTIMEOUT: %u seconds", sec);
        setsockopt(state->sockfd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, &sec, sizeof(sec));
#elif defined(TCP_KEEPINIT)
        FF_DEBUG("Using TCP_KEEPINIT: %u seconds", sec);
        setsockopt(state->sockfd, IPPROTO_TCP, TCP_KEEPINIT, &sec, sizeof(sec));
#elif defined(TCP_USER_TIMEOUT)
        FF_DEBUG("Using TCP_USER_TIMEOUT: %u milliseconds", state->timeout);
        setsockopt(state->sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &state->timeout, sizeof(state->timeout));
#else
        FF_DEBUG("Current platform does not support TCP connection timeout");
#endif
    }

    return nullptr;

error:
    FF_DEBUG("Error occurred during initialization");
    if (state->addr != nullptr) {
        FF_DEBUG("Releasing address information");
        freeaddrinfo(state->addr);
        state->addr = nullptr;
    }

    if (state->sockfd > 0) {
        FF_DEBUG("Closing socket: fd=%d", state->sockfd);
        close(state->sockfd);
        state->sockfd = -1;
    }

    ffStrbufClear(&state->command);
    return ret;
}

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, uint16_t port, const char* path, const char* headers) {
    FF_DEBUG("Preparing to send HTTP request: host=%s, port=%u, path=%s", host, port, path);

    if (state->compression) {
        FF_DEBUG("Compression enabled, checking if zlib is available");

#ifdef FF_HAVE_ZLIB
        const char* zlibError = ffNetworkingLoadZlibLibrary();
        // Only enable compression if zlib library is successfully loaded
        if (zlibError == nullptr) {
            FF_DEBUG("Successfully loaded zlib library, compression enabled");
        } else {
            FF_DEBUG("Failed to load zlib library, compression disabled: %s", zlibError);
            state->compression = false;
        }
#else
        FF_DEBUG("zlib not supported at build time, compression disabled");
        state->compression = false;
#endif
    } else {
        FF_DEBUG("Compression disabled");
    }

    const char* initResult = initNetworkingState(state, host, port, path, headers);
    if (initResult != nullptr) {
        FF_DEBUG("Initialization failed: %s", initResult);
        return initResult;
    }
    FF_DEBUG("Network state initialization successful");

    const char* tfoResult = tryNonThreadingFastPath(state);
    if (tfoResult == nullptr) {
        FF_DEBUG("TryNonThreadingFastPath() succeeded or in progress");
        return nullptr;
    }
    FF_DEBUG("TryNonThreadingFastPath() failed: %s, trying traditional connection", tfoResult);

#ifdef FF_HAVE_THREADS
    if (instance.config.general.multithreading) {
        FF_DEBUG("Multithreading mode enabled, creating connection thread");
        state->thread = ffThreadCreate(connectAndSendThreadMain, state);
        if (state->thread) {
            FF_DEBUG("Thread creation successful: thread=%p", (void*) (uintptr_t) state->thread);
            return nullptr;
        }
        FF_DEBUG("Thread creation failed");
    } else {
        FF_DEBUG("Multithreading mode disabled, connecting in main thread");
    }
#endif

    return connectAndSend(state);
}

const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer) {
    assert(buffer->allocated > 0);
    FF_DEBUG("Preparing to receive HTTP response");
    uint32_t timeout = state->timeout;

#ifdef FF_HAVE_THREADS
    if (state->thread) {
        FF_DEBUG("Connection thread is running, waiting for it to complete (timeout=%u ms)", timeout);
        if (!ffThreadJoin(state->thread, timeout)) {
            FF_DEBUG("Thread join failed or timed out");
            return "ffThreadJoin() failed or timeout";
        }
        FF_DEBUG("Thread completed successfully");
        state->thread = 0;
    }
#endif

    if (state->sockfd == -1) {
        FF_DEBUG("Invalid socket, HTTP request might have failed");
        return "ffNetworkingSendHttpRequest() failed";
    }

    // Set larger initial receive buffer instead of small repeated receives
    int rcvbuf = 64 * 1024;
    setsockopt(state->sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    // The timeout has to be enforced by the socket itself on every platform: the poll() below
    // only reports readability once, so a server that sends a partial response and then keeps
    // the connection open would otherwise block this loop forever.
    if (timeout > 0) {
        FF_DEBUG("Setting receive timeout: %u ms", timeout);
        struct timeval timev;
        timev.tv_sec = timeout / 1000;
        timev.tv_usec = (typeof(timev.tv_usec)) ((timeout % 1000) * 1000); // milliseconds to microseconds
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    }

#ifdef __APPLE__
    // poll for the socket to be readable.
    // Because of the non-blocking connectx() call, the connection might not be established yet
    FF_DEBUG("Using poll() to check if socket is readable");
    {
        int pollRes = poll(&(struct pollfd) {
                               .fd = state->sockfd,
                               .events = POLLIN },
            1,
            timeout > 0 ? (int) timeout : -1);
        if (pollRes == 0) {
            FF_DEBUG("poll() timed out after %u ms", timeout);
            close(state->sockfd);
            state->sockfd = -1;
            return "poll() timeout";
        } else if (pollRes == -1) {
            FF_DEBUG("poll() failed: %s", strerror(errno));
            close(state->sockfd);
            state->sockfd = -1;
            return "poll() failed";
        }
    }
    FF_DEBUG("Socket is readable, proceeding to receive data");
#endif

    if (shutdown(state->sockfd, SHUT_WR) == -1) {
        FF_DEBUG("Failed to shutdown socket send: %s", strerror(errno));
        // Not a critical error, continue anyway
    }

    FF_DEBUG("Starting data reception");
    [[maybe_unused]] int recvCount = 0;
    uint32_t contentLength = 0;
    uint32_t headerEnd = 0;
    bool chunked = false;

    // Runs until the response is framed; the buffer is grown on demand at the top
    for (;;) {
        if (ffStrbufGetFree(buffer) == 0) {
            // `Content-Length` may be absent (e.g. chunked responses). Grow the buffer
            // on demand instead of silently truncating the response.
            if (buffer->allocated >= FF_NETWORKING_MAX_RESPONSE_SIZE) {
                FF_DEBUG("Response is too large: %u bytes, aborting", buffer->allocated);
                close(state->sockfd);
                state->sockfd = -1;
                return "Response too large";
            }
            FF_DEBUG("Receive buffer is full, extending it");
            // Asking for exactly the room that is left under the cap, rather than for as much as
            // the buffer holds again: the allocation is rounded up to a power of two, so a doubling
            // from a size that is not one of those lands past the cap the check above just cleared.
            ffStrbufEnsureFreeNoCheck(buffer, FF_NETWORKING_MAX_RESPONSE_SIZE - buffer->length - 1);
        }

        // When the remaining length is known, ask for exactly that much. MSG_WAITALL then
        // returns as soon as the response is complete, instead of waiting for the server
        // to close the connection.
        // Without a Content-Length the requested length is just "whatever fits", so
        // waiting for all of it would block until the server closes -- and the framing
        // checks below would never get a chance to run. Read whatever has arrived instead
        // and let those checks decide when the response is complete.
        uint32_t want = ffStrbufGetFree(buffer);
        int recvFlags = 0;
        if (contentLength > 0 && headerEnd > 0) {
            uint32_t remaining = headerEnd + 4 + contentLength - buffer->length;
            if (remaining < want) {
                want = remaining;
            }
            recvFlags = MSG_WAITALL;
        }

        FF_DEBUG("Data reception loop #%d, current buffer size: %u, requesting %u bytes",
            ++recvCount,
            buffer->length,
            want);

        ssize_t received = recv(state->sockfd, buffer->chars + buffer->length, want, recvFlags);

        if (received <= 0) {
            if (received == 0) {
                FF_DEBUG("Connection closed (received=0)");
            } else {
                FF_DEBUG("Reception failed: %s", strerror(errno));
            }
            break;
        }

        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';

        FF_DEBUG("Successfully received %zd bytes of data, total: %u bytes", received, buffer->length);

        // Check if HTTP header end marker is found
        if (headerEnd == 0) {
            char* pHeaderEnd = memmem(buffer->chars, buffer->length, "\r\n\r\n", 4);
            if (pHeaderEnd) {
                headerEnd = (uint32_t) (pHeaderEnd - buffer->chars);
                FF_DEBUG("Found HTTP header end marker, position: %u", headerEnd);

                // Check for Content-Length header to pre-allocate enough memory
                uint32_t valueLen = 0;
                const char* clHeader = ffNetworkingFindHeader(buffer->chars, headerEnd, "Content-Length:", &valueLen);
                if (clHeader) {
                    contentLength = (uint32_t) strtoul(clHeader, nullptr, 10);
                    if (contentLength > 0) {
                        if (contentLength > FF_NETWORKING_MAX_RESPONSE_SIZE) { // 1MB limit to prevent excessive memory allocation and potential attacks
                            FF_DEBUG("Content-Length is too large: %u bytes, aborting", contentLength);
                            close(state->sockfd);
                            state->sockfd = -1;
                            return "Content-Length too large";
                        }

                        FF_DEBUG("Detected Content-Length: %u, pre-allocating buffer", contentLength);
                        // Ensure buffer is large enough, adding header size and some margin
                        ffStrbufEnsureFree(buffer, contentLength + 16);
                        FF_DEBUG("Extended receive buffer to %u bytes", buffer->allocated);
                    }
                }

                // A chunked response has no Content-Length; it is framed by a last-chunk
                const char* teHeader = ffNetworkingFindHeader(buffer->chars, headerEnd, "Transfer-Encoding:", &valueLen);
                if (teHeader != nullptr) {
                    switch (ffNetworkingParseTransferEncoding(teHeader, valueLen)) {
                        case FF_NETWORKING_TE_CHUNKED:
                            FF_DEBUG("Detected chunked transfer encoding");
                            chunked = true;
                            break;
                        case FF_NETWORKING_TE_UNSUPPORTED:
                            // The framing of e.g. `gzip, chunked` is unreadable and the payload
                            // would stay encoded, so fail instead of returning garbage
                            FF_DEBUG("Unsupported Transfer-Encoding: %.*s", (int) valueLen, teHeader);
                            close(state->sockfd);
                            state->sockfd = -1;
                            return "Unsupported Transfer-Encoding";
                        default:
                            break;
                    }
                }
            }
        }

        // Stop as soon as the response is framed, rather than waiting for the FIN
        if (chunked) {
            uint32_t consumed = 0;
            int complete = ffNetworkingChunkedComplete(buffer->chars + headerEnd + 4, buffer->length - headerEnd - 4, &consumed);
            if (complete < 0) {
                FF_DEBUG("Malformed chunked body");
                close(state->sockfd);
                state->sockfd = -1;
                return "Malformed chunked body";
            }
            if (complete > 0) {
                FF_DEBUG("Chunked body complete, %u bytes of encoded body", consumed);
                break;
            }
        } else if (contentLength > 0 && buffer->length >= headerEnd + 4 + contentLength) {
            break;
        }
    }

    FF_DEBUG("Closing socket: fd=%d", state->sockfd);
    close(state->sockfd);
    state->sockfd = -1;

    if (buffer->length == 0) {
        FF_DEBUG("Server response is empty");
        return "Empty server response received";
    }

    if (headerEnd == 0) {
        FF_DEBUG("No HTTP header end marker found");
        return "No HTTP header end found";
    }

    if (chunked && !ffNetworkingDecodeChunked(buffer, &headerEnd)) {
        return "Failed to decode chunked response";
    }

    if (!ffStrbufStartsWithS(buffer, "HTTP/1.0 200 OK\r\n") && !ffStrbufStartsWithS(buffer, "HTTP/1.1 200 OK\r\n")) {
        FF_DEBUG("Invalid response: %.40s...", buffer->chars);
        return "Invalid response";
    }
    FF_DEBUG("Received valid HTTP 200 response, content %u bytes, total %u bytes", contentLength, buffer->length);

    // A chunked response was framed by its last-chunk and has been rewritten around it, so any
    // `Content-Length` the server also sent describes a body that no longer exists. RFC 9112 6.3
    // says not to send both; when one does, the chunked framing is the one that was acted on.
    if (!chunked && contentLength > 0 && buffer->length != contentLength + headerEnd + 4) {
        FF_DEBUG("Received content length mismatches: %u != %u", buffer->length, contentLength + headerEnd + 4);
        return "Content length mismatch";
    }

// If compression was used, try to decompress
#ifdef FF_HAVE_ZLIB
    if (state->compression) {
        FF_DEBUG("Content received, checking if compressed");
        if (!ffNetworkingDecompressGzip(buffer, buffer->chars + headerEnd)) {
            FF_DEBUG("Decompression failed or invalid compression format");
            return "Failed to decompress or invalid format";
        } else {
            FF_DEBUG("Decompression successful or no decompression needed, total length after decompression: %u bytes", buffer->length);
        }
    }
#endif

    return nullptr;
}
