#include <mswsock.h>
#include <ws2tcpip.h>

// Must be included after <mswsock.h>
#include "fastfetch.h"
#include "common/networking.h"
#include "common/strutil.h"
#include "common/debug.h"

static LPFN_CONNECTEX ConnectEx;

// Upper bound of a single HTTP response, guarding against excessive memory allocation
#define FF_NETWORKING_MAX_RESPONSE_SIZE (1024u * 1024u)

static const char* initWsaData(WSADATA* wsaData) {
    FF_DEBUG("Initializing WinSock");
    if (WSAStartup(MAKEWORD(2, 2), wsaData) != 0) {
        FF_DEBUG("WSAStartup() failed");
        return "WSAStartup() failed";
    }

    if (LOBYTE(wsaData->wVersion) != 2 || HIBYTE(wsaData->wVersion) != 2) {
        FF_DEBUG("Invalid wsaData version found: %d.%d", LOBYTE(wsaData->wVersion), HIBYTE(wsaData->wVersion));
        WSACleanup();
        return "Invalid wsaData version found";
    }

    // Dummy socket needed for WSAIoctl
    SOCKET sockfd = WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, 0);
    if (sockfd == INVALID_SOCKET) {
        FF_DEBUG("WSASocketW(AF_INET, SOCK_STREAM) failed");
        WSACleanup();
        return "WSASocketW(AF_INET, SOCK_STREAM) failed";
    }

    DWORD dwBytes;
    GUID guid = WSAID_CONNECTEX;
    if (WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &ConnectEx, sizeof(ConnectEx), &dwBytes, nullptr, nullptr) != 0) {
        FF_DEBUG("WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER) failed");
        closesocket(sockfd);
        WSACleanup();
        return "WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER) failed";
    }

    closesocket(sockfd);
    FF_DEBUG("WinSock initialized successfully");

    return nullptr;
}

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, uint16_t port, const char* path, const char* headers) {
    FF_DEBUG("Preparing to send HTTP request: host=%s, port=%u, path=%s", host, port, path);

    if (state->compression) {
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

    static WSADATA wsaData;
    if (wsaData.wVersion == 0) {
        const char* error = initWsaData(&wsaData);
        if (error != nullptr) {
            wsaData.wVersion = (WORD) -1;
            FF_DEBUG("WinSock initialization failed: %s", error);
            return error;
        }
    } else if (wsaData.wVersion == (WORD) -1) {
        FF_DEBUG("WinSock initialization previously failed");
        return "initWsaData() failed before";
    }

    ADDRINFOW* addr;
    ADDRINFOW hints = {
        .ai_flags = AI_NUMERICSERV,
        .ai_family = state->ipv6 ? AF_INET6 : AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    wchar_t hostW[256];
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(hostW, (ULONG) sizeof(hostW), nullptr, host, (ULONG) strlen(host) + 1))) {
        FF_DEBUG("Failed to convert host to wide string: %s", host);
        return "Failed to convert host to wide string";
    }

    wchar_t portW[6];
    _itow(port, portW, 10);

    FF_DEBUG("Resolving address: %s:%u (%s)", host, port, state->ipv6 ? "IPv6" : "IPv4");
    if (GetAddrInfoW(hostW, portW, &hints, &addr) != 0) {
        FF_DEBUG("GetAddrInfoW() failed");
        return "GetAddrInfoW() failed";
    }

    state->sockfd = WSASocketW(addr->ai_family, addr->ai_socktype, addr->ai_protocol, nullptr, 0, 0);
    if (state->sockfd == INVALID_SOCKET) {
        FF_DEBUG("WSASocketW() failed");
        FreeAddrInfoW(addr);
        return "WSASocketW() failed";
    }

    DWORD flag = 1;
#ifdef TCP_NODELAY
    // Enable TCP_NODELAY to disable Nagle's algorithm
    if (setsockopt(state->sockfd, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(flag)) != 0) {
        FF_DEBUG("Failed to set TCP_NODELAY: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
    } else {
        FF_DEBUG("Successfully disabled Nagle's algorithm");
    }
#endif

    // Set timeout if needed
    if (state->timeout > 0) {
        FF_DEBUG("Setting connection timeout: %u ms", state->timeout);
        setsockopt(state->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &state->timeout, sizeof(state->timeout));
    }

    // ConnectEx requires the socket to be initially bound
    if ((state->ipv6
                ? bind(state->sockfd, (SOCKADDR*) &(struct sockaddr_in6) {
                                          .sin6_family = AF_INET6,
                                          .sin6_addr = in6addr_any,
                                      },
                      sizeof(struct sockaddr_in6))
                : bind(state->sockfd, (SOCKADDR*) &(struct sockaddr_in) {
                                          .sin_family = AF_INET,
                                          .sin_addr.s_addr = INADDR_ANY,
                                      },
                      sizeof(struct sockaddr_in))) != 0) {
        FF_DEBUG("bind() failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        closesocket(state->sockfd);
        FreeAddrInfoW(addr);
        state->sockfd = INVALID_SOCKET;
        return "bind() failed";
    }

    // Initialize overlapped structure with WSA event for asynchronous I/O
    state->overlapped = (OVERLAPPED) {};

    if (!NT_SUCCESS(NtCreateEvent(&state->overlapped.hEvent, EVENT_ALL_ACCESS, nullptr, NotificationEvent, FALSE))) {
        FF_DEBUG("NtCreateEvent() failed");
        closesocket(state->sockfd);
        FreeAddrInfoW(addr);
        state->sockfd = INVALID_SOCKET;
        return "NtCreateEvent() failed";
    }

    // Build HTTP command
    ffStrbufInitA(&state->command, 128);
    ffStrbufAppendS(&state->command, "GET ");
    ffStrbufAppendS(&state->command, path);
    ffStrbufAppendS(&state->command, " HTTP/1.0\r\nHost: ");
    if (strchr(host, ':') != nullptr) {
        // An IPv6 literal has to be bracketed in the Host header (RFC 9110 7.2), while
        // GetAddrInfoW() wants it bare
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
    ffStrbufAppendS(&state->command, "\r\nConnection: close\r\n"); // Explicitly request connection closure

    // Add compression support if enabled
    if (state->compression) {
        FF_DEBUG("Enabling HTTP content compression");
        ffStrbufAppendS(&state->command, "Accept-Encoding: gzip\r\n");
    }

    ffStrbufAppendS(&state->command, headers);
    ffStrbufAppendS(&state->command, "\r\n");

#ifdef TCP_FASTOPEN
    if (state->tfo) {
        // Set TCP Fast Open
        flag = 1;
        if (setsockopt(state->sockfd, IPPROTO_TCP, TCP_FASTOPEN, (char*) &flag, sizeof(flag)) != 0) {
            FF_DEBUG("Failed to set TCP_FASTOPEN option: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        } else {
            FF_DEBUG("Successfully set TCP_FASTOPEN option");
        }
    } else {
        FF_DEBUG("TCP Fast Open disabled");
    }
#endif

    FF_DEBUG("Using ConnectEx to send %u bytes of data", state->command.length);
    DWORD sent = 0;
    BOOL result = ConnectEx(state->sockfd, addr->ai_addr, (int) addr->ai_addrlen, state->command.chars, state->command.length, &sent, &state->overlapped);

    FreeAddrInfoW(addr);
    addr = nullptr;

    if (!result) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            FF_DEBUG("ConnectEx() failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
            NtClose(state->overlapped.hEvent);
            closesocket(state->sockfd);
            state->sockfd = INVALID_SOCKET;
            ffStrbufDestroy(&state->command);
            return "ConnectEx() failed";
        } else {
            FF_DEBUG("ConnectEx() pending");
        }
    } else {
        FF_DEBUG("ConnectEx() succeeded, sent %u bytes of data", (unsigned) sent);
    }

    // No need to cleanup state fields here since we need them in the receive function
    return nullptr;
}

const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer) {
    assert(buffer->allocated > 0);
    FF_DEBUG("Preparing to receive HTTP response");

    if (state->sockfd == INVALID_SOCKET) {
        FF_DEBUG("Invalid socket, HTTP request might have failed");
        return "ffNetworkingSendHttpRequest() failed";
    }

    DWORD transfer;
    uint32_t timeout = state->timeout;
    if (!GetOverlappedResultEx((HANDLE) state->sockfd, &state->overlapped, &transfer, timeout > 0 ? timeout : INFINITE, FALSE)) {
        DWORD error = GetLastError();
        if (error == WAIT_TIMEOUT) {
            FF_DEBUG("GetOverlappedResultEx timed out");
        } else {
            FF_DEBUG("GetOverlappedResultEx failed: %s", ffDebugWin32Error(error));
        }
        IO_STATUS_BLOCK cancelIosb = {};
        if (NT_SUCCESS(NtCancelIoFileEx((HANDLE) state->sockfd, (PIO_STATUS_BLOCK) &state->overlapped, &cancelIosb))) {
            NtWaitForSingleObject(state->overlapped.hEvent, TRUE, &(LARGE_INTEGER) { .QuadPart = (int64_t) 10 * -10000 });
        }
        NtClose(state->overlapped.hEvent);
        closesocket(state->sockfd);
        ffStrbufDestroy(&state->command);
        return "GetOverlappedResultEx() failed or timeout";
    }
    FF_DEBUG("GetOverlappedResultEx succeeded, %u bytes sent", (unsigned) transfer);
    ffStrbufDestroy(&state->command);
    NtClose(state->overlapped.hEvent);
    state->overlapped.hEvent = nullptr;

    if (setsockopt(state->sockfd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0) != 0) {
        FF_DEBUG("Failed to update connect context: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        // Not a critical error, continue anyway
    }

    if (shutdown(state->sockfd, SD_SEND) == SOCKET_ERROR) {
        FF_DEBUG("Failed to shutdown socket send: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        // Not a critical error, continue anyway
    }

    if (timeout > 0) {
        FF_DEBUG("Setting receive timeout: %u ms", timeout);
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));
    }

    // Set larger receive buffer for better performance
    int rcvbuf = 65536; // 64KB
    if (setsockopt(state->sockfd, SOL_SOCKET, SO_RCVBUF, (const char*) &rcvbuf, sizeof(rcvbuf))) {
        FF_DEBUG("Failed to set SO_RCVBUF: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
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
                closesocket(state->sockfd);
                state->sockfd = INVALID_SOCKET;
                return "Response too large";
            }
            FF_DEBUG("Receive buffer is full, extending it");
            // Asking for exactly the room that is left under the cap, rather than for as much as
            // the buffer holds again: the allocation is rounded up to a power of two, so a doubling
            // from a size that is not one of those lands past the cap the check above just cleared.
            ffStrbufEnsureFreeNoCheck(buffer, FF_NETWORKING_MAX_RESPONSE_SIZE - buffer->length - 1);
        }

        FF_DEBUG("Data reception loop #%d, current buffer size: %u, available space: %u",
            ++recvCount,
            buffer->length,
            ffStrbufGetFree(buffer));

        DWORD received = 0, recvFlags = 0;
        int recvResult = WSARecv(state->sockfd, &(WSABUF) {
                                                    .buf = buffer->chars + buffer->length,
                                                    .len = (ULONG) ffStrbufGetFree(buffer),
                                                },
            1,
            &received,
            &recvFlags,
            nullptr,
            nullptr);

        if (recvResult == SOCKET_ERROR || received == 0) {
            if (recvResult == 0 && received == 0) {
                FF_DEBUG("Connection closed (received=0)");
            } else {
                FF_DEBUG("Reception failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
            }
            break;
        }

        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';

        FF_DEBUG("Successfully received %u bytes of data, total: %u bytes", (unsigned) received, buffer->length);

        // Check if HTTP header end marker is found
        if (headerEnd == 0) {
            char* pHeaderEnd = strstr(buffer->chars, "\r\n\r\n");
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
                            closesocket(state->sockfd);
                            state->sockfd = INVALID_SOCKET;
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
                            closesocket(state->sockfd);
                            state->sockfd = INVALID_SOCKET;
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
                closesocket(state->sockfd);
                state->sockfd = INVALID_SOCKET;
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

    FF_DEBUG("Closing socket: fd=%u", (unsigned) state->sockfd);
    closesocket(state->sockfd);
    state->sockfd = INVALID_SOCKET;

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
    FF_DEBUG("Received valid HTTP 200 response, content length: %u bytes, total length: %u bytes",
        contentLength,
        buffer->length);

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
