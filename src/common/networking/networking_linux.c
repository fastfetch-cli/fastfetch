#include "fastfetch.h"
#include "common/networking/networking.h"
#include "common/time.h"
#include "common/library.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"
#include "util/debug.h"

#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h> // For FreeBSD
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>

static const char* tryNonThreadingFastPath(FFNetworkingState* state)
{
    #if defined(TCP_FASTOPEN) || __APPLE__

        if (!state->tfo)
        {
            #if __linux__ || __GNU__
            // Linux doesn't support sendto() on unconnected sockets
            FF_DEBUG("TCP Fast Open disabled, skipping");
            return "TCP Fast Open disabled";
            #endif
        }
        else
        {
            FF_DEBUG("Attempting to use TCP Fast Open to connect");

            #ifndef __APPLE__ // On macOS, TCP_FASTOPEN doesn't seem to be needed
            // Set TCP Fast Open
            #if __linux__ || __GNU__
            int flag = 5; // the queue length of pending packets
            #else
            int flag = 1; // enable TCP Fast Open
            #endif
            if (setsockopt(state->sockfd, IPPROTO_TCP,
                #ifdef __APPLE__
                // https://github.com/rust-lang/libc/pull/3135
                0x218 // TCP_FASTOPEN_FORCE_ENABLE
                #else
                TCP_FASTOPEN
                #endif
                , &flag, sizeof(flag)) != 0) {
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
        FF_DEBUG("Using sendto() + MSG_DONTWAIT to send %u bytes of data", state->command.length);
        ssize_t sent = sendto(state->sockfd,
                                state->command.chars,
                                state->command.length,
            #ifdef MSG_FASTOPEN
                                MSG_FASTOPEN |
            #endif
            #ifdef MSG_NOSIGNAL
                                MSG_NOSIGNAL |
            #endif
                                MSG_DONTWAIT,
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
            SAE_ASSOCID_ANY, state->tfo ? CONNECT_DATA_IDEMPOTENT : 0,
            &(struct iovec) {
                .iov_base = state->command.chars,
                .iov_len = state->command.length,
            }, 1, &sent, NULL) != 0) sent = 0;
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
        ))
        {
            FF_DEBUG(
                #ifdef __APPLE__
                "connectx()"
                #else
                "sendto()"
                #endif
                " %s (sent=%zd, errno=%d: %s)", errno == 0 ? "succeeded" : "was in progress",
                sent, errno, strerror(errno));
            freeaddrinfo(state->addr);
            state->addr = NULL;
            ffStrbufDestroy(&state->command);
            return NULL;
        }

        FF_DEBUG(
            #ifdef __APPLE__
            "connectx()"
            #else
            "sendto()"
            #endif
            " failed: %s (errno=%d)", strerror(errno), errno);
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
static const char* connectAndSend(FFNetworkingState* state)
{
    const char* ret = NULL;
    FF_DEBUG("Using traditional connection method to connect");

    FF_DEBUG("Attempting connect() to server...");
    if(connect(state->sockfd, state->addr->ai_addr, state->addr->ai_addrlen) == -1)
    {
        FF_DEBUG("connect() failed: %s (errno=%d)", strerror(errno), errno);
        ret = "connect() failed";
        goto error;
    }
    FF_DEBUG("connect() succeeded");

    FF_DEBUG("Attempting to send %u bytes of data...", state->command.length);
    if(send(state->sockfd, state->command.chars, state->command.length, 0) < 0)
    {
        FF_DEBUG("send() failed: %s (errno=%d)", strerror(errno), errno);
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
    state->addr = NULL;
    ffStrbufDestroy(&state->command);

    return ret;
}

FF_THREAD_ENTRY_DECL_WRAPPER(connectAndSend, FFNetworkingState*);

// Parallel DNS resolution and socket creation
static const char* initNetworkingState(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    FF_DEBUG("Initializing network connection state: host=%s, path=%s", host, path);

    // Initialize command and host information
    ffStrbufInitA(&state->command, 64);
    ffStrbufAppendS(&state->command, "GET ");
    ffStrbufAppendS(&state->command, path);
    ffStrbufAppendS(&state->command, " HTTP/1.0\nHost: ");
    ffStrbufAppendS(&state->command, host);
    ffStrbufAppendS(&state->command, "\r\n");

    // Add extra optimized HTTP headers
    ffStrbufAppendS(&state->command, "Connection: close\r\n"); // Explicitly tell the server we don't need to keep the connection

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

    const char* ret = NULL;

    struct addrinfo hints = {
        .ai_family = state->ipv6 ? AF_INET6 : AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_NUMERICSERV
    };

    FF_DEBUG("Resolving address: %s (%s)", host, state->ipv6 ? "IPv6" : "IPv4");
    // Use AI_NUMERICSERV flag to indicate the service is a numeric port, reducing parsing time

    if(getaddrinfo(host, "80", &hints, &state->addr) != 0)
    {
        FF_DEBUG("getaddrinfo() failed: %s", gai_strerror(errno));
        ret = "getaddrinfo() failed";
        goto error;
    }
    FF_DEBUG("Address resolution successful");

    FF_DEBUG("Creating socket");
    state->sockfd = socket(state->addr->ai_family, state->addr->ai_socktype, state->addr->ai_protocol);
    if(state->sockfd == -1)
    {
        FF_DEBUG("socket() failed: %s (errno=%d)", strerror(errno), errno);
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

    if (state->timeout > 0)
    {
        FF_DEBUG("Setting connection timeout: %u ms", state->timeout);
        FF_MAYBE_UNUSED uint32_t sec = state->timeout / 1000;
        if (sec == 0) sec = 1;

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

    return NULL;

error:
    FF_DEBUG("Error occurred during initialization");
    if (state->addr != NULL)
    {
        FF_DEBUG("Releasing address information");
        freeaddrinfo(state->addr);
        state->addr = NULL;
    }

    if (state->sockfd > 0)
    {
        FF_DEBUG("Closing socket: fd=%d", state->sockfd);
        close(state->sockfd);
        state->sockfd = -1;
    }
    return ret;
}

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    FF_DEBUG("Preparing to send HTTP request: host=%s, path=%s", host, path);

    if (state->compression)
    {
        FF_DEBUG("Compression enabled, checking if zlib is available");

        #ifdef FF_HAVE_ZLIB
        const char* zlibError = ffNetworkingLoadZlibLibrary();
        // Only enable compression if zlib library is successfully loaded
        if (zlibError == NULL)
        {
            FF_DEBUG("Successfully loaded zlib library, compression enabled");
        } else {
            FF_DEBUG("Failed to load zlib library, compression disabled: %s", zlibError);
            state->compression = false;
        }
        #else
        FF_DEBUG("zlib not supported at build time, compression disabled");
        state->compression = false;
        #endif
    }
    else
    {
        FF_DEBUG("Compression disabled");
    }

    const char* initResult = initNetworkingState(state, host, path, headers);
    if (initResult != NULL) {
        FF_DEBUG("Initialization failed: %s", initResult);
        return initResult;
    }
    FF_DEBUG("Network state initialization successful");

    const char* tfoResult = tryNonThreadingFastPath(state);
    if (tfoResult == NULL) {
        FF_DEBUG("TryNonThreadingFastPath() succeeded or in progress");
        return NULL;
    }
    FF_DEBUG("TryNonThreadingFastPath() failed: %s, trying traditional connection", tfoResult);

    #ifdef FF_HAVE_THREADS
    if (instance.config.general.multithreading)
    {
        FF_DEBUG("Multithreading mode enabled, creating connection thread");
        state->thread = ffThreadCreate(connectAndSendThreadMain, state);
        if (state->thread) {
            FF_DEBUG("Thread creation successful: thread=%p", (void*)(uintptr_t)state->thread);
            return NULL;
        }
        FF_DEBUG("Thread creation failed");
    } else {
        FF_DEBUG("Multithreading mode disabled, connecting in main thread");
    }
    #endif

    return connectAndSend(state);
}

const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer)
{
    FF_DEBUG("Preparing to receive HTTP response");
    uint32_t timeout = state->timeout;

    #ifdef FF_HAVE_THREADS
    if (state->thread)
    {
        FF_DEBUG("Connection thread is running, waiting for it to complete (timeout=%u ms)", timeout);
        if (!ffThreadJoin(state->thread, timeout)) {
            FF_DEBUG("Thread join failed or timed out");
            return "ffThreadJoin() failed or timeout";
        }
        FF_DEBUG("Thread completed successfully");
        state->thread = 0;
    }
    #endif

    if(state->sockfd == -1)
    {
        FF_DEBUG("Invalid socket, HTTP request might have failed");
        return "ffNetworkingSendHttpRequest() failed";
    }

    // Set larger initial receive buffer instead of small repeated receives
    int rcvbuf = 65536; // 64KB
    setsockopt(state->sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    #ifdef __APPLE__
    // poll for the socket to be readable.
    // Because of the non-blocking connectx() call, the connection might not be established yet
    FF_DEBUG("Using poll() to check if socket is readable");
    if (poll(&(struct pollfd) {
        .fd = state->sockfd,
        .events = POLLIN
    }, 1, timeout > 0 ? (int) timeout : -1) == -1)
    {
        FF_DEBUG("poll() failed: %s (errno=%d)", strerror(errno), errno);
        close(state->sockfd);
        state->sockfd = -1;
        return "poll() failed";
    }
    FF_DEBUG("Socket is readable, proceeding to receive data");
    #else
    if(timeout > 0)
    {
        FF_DEBUG("Setting receive timeout: %u ms", timeout);
        struct timeval timev;
        timev.tv_sec = timeout / 1000;
        timev.tv_usec = (__typeof__(timev.tv_usec)) ((timeout % 1000) * 1000); //milliseconds to microseconds
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    }
    #endif

    FF_DEBUG("Starting data reception");
    FF_MAYBE_UNUSED int recvCount = 0;
    uint32_t contentLength = 0;
    char* headerEnd = NULL;

    do {
        FF_DEBUG("Data reception loop #%d, current buffer size: %u, available space: %u",
                 ++recvCount, buffer->length, ffStrbufGetFree(buffer));

        ssize_t received = recv(state->sockfd, buffer->chars + buffer->length, ffStrbufGetFree(buffer), MSG_WAITALL);

        if (received <= 0) {
            if (received == 0) {
                FF_DEBUG("Connection closed (received=0)");
            } else {
                FF_DEBUG("Reception failed: %s (errno=%d)", strerror(errno), errno);
            }
            break;
        }

        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';

        FF_DEBUG("Successfully received %zd bytes of data, total: %u bytes", received, buffer->length);

        // Check if HTTP header end marker is found
        if (headerEnd == NULL) {
            headerEnd = memmem(buffer->chars, buffer->length, "\r\n\r\n", 4);
            if (headerEnd != NULL) {
                FF_DEBUG("Found HTTP header end marker, position: %ld", (long)(headerEnd - buffer->chars));

                // Check for Content-Length header to pre-allocate enough memory
                const char* clHeader = strcasestr(buffer->chars, "Content-Length:");
                if (clHeader) {
                    contentLength = (uint32_t) strtoul(clHeader + 16, NULL, 10);
                    if (contentLength > 0) {
                        FF_DEBUG("Detected Content-Length: %u, pre-allocating buffer", contentLength);
                        // Ensure buffer is large enough, adding header size and some margin
                        ffStrbufEnsureFree(buffer, contentLength + 16);
                        FF_DEBUG("Extended receive buffer to %u bytes", buffer->length);
                    }
                }
            }
        }
    } while (ffStrbufGetFree(buffer) > 0);

    FF_DEBUG("Closing socket: fd=%d", state->sockfd);
    close(state->sockfd);
    state->sockfd = -1;

    if (buffer->length == 0) {
        FF_DEBUG("Server response is empty");
        return "Empty server response received";
    }

    if (headerEnd == NULL) {
        FF_DEBUG("No HTTP header end marker found");
        return "No HTTP header end found";
    }
    if (contentLength > 0 && buffer->length != contentLength + (uint32_t)(headerEnd - buffer->chars) + 4) {
        FF_DEBUG("Received content length mismatches: %u != %u", buffer->length, contentLength + (uint32_t)(headerEnd - buffer->chars) + 4);
        return "Content length mismatch";
    }

    if (ffStrbufStartsWithS(buffer, "HTTP/1.0 200 OK\r\n")) {
        FF_DEBUG("Received valid HTTP 200 response, content %u bytes, total %u bytes", contentLength, buffer->length);
    } else {
        FF_DEBUG("Invalid response: %.40s...", buffer->chars);
        return "Invalid response";
    }

    // If compression was used, try to decompress
    #ifdef FF_HAVE_ZLIB
    if (state->compression) {
        FF_DEBUG("Content received, checking if compressed");
        if (!ffNetworkingDecompressGzip(buffer, headerEnd)) {
            FF_DEBUG("Decompression failed or invalid compression format");
            return "Failed to decompress or invalid format";
        } else {
            FF_DEBUG("Decompression successful or no decompression needed, total length after decompression: %u bytes", buffer->length);
        }
    }
    #endif

    return NULL;
}
