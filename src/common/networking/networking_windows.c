#include <mswsock.h>
#include <ws2tcpip.h>

//Must be included after <mswsock.h>
#include "fastfetch.h"
#include "common/networking/networking.h"
#include "util/stringUtils.h"
#include "util/debug.h"

static LPFN_CONNECTEX ConnectEx;

static const char* initWsaData(WSADATA* wsaData)
{
    FF_DEBUG("Initializing WinSock");
    if(WSAStartup(MAKEWORD(2, 2), wsaData) != 0) {
        FF_DEBUG("WSAStartup() failed");
        return "WSAStartup() failed";
    }

    if(LOBYTE(wsaData->wVersion) != 2 || HIBYTE(wsaData->wVersion) != 2) {
        FF_DEBUG("Invalid wsaData version found: %d.%d", LOBYTE(wsaData->wVersion), HIBYTE(wsaData->wVersion));
        return "Invalid wsaData version found";
    }

    //Dummy socket needed for WSAIoctl
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == INVALID_SOCKET) {
        FF_DEBUG("socket(AF_INET, SOCK_STREAM) failed");
        return "socket(AF_INET, SOCK_STREAM) failed";
    }

    DWORD dwBytes;
    GUID guid = WSAID_CONNECTEX;
    if(WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guid, sizeof(guid),
                    &ConnectEx, sizeof(ConnectEx),
                    &dwBytes, NULL, NULL) != 0) {
        FF_DEBUG("WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER) failed");
        return "WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER) failed";
    }

    closesocket(sockfd);
    FF_DEBUG("WinSock initialized successfully");

    return NULL;
}

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    FF_DEBUG("Preparing to send HTTP request: host=%s, path=%s", host, path);

    if (state->compression)
    {
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

    static WSADATA wsaData;
    if (wsaData.wVersion == 0)
    {
        const char* error = initWsaData(&wsaData);
        if (error != NULL)
        {
            wsaData.wVersion = (WORD) -1;
            FF_DEBUG("WinSock initialization failed: %s", error);
            return error;
        }
    }
    else if (wsaData.wVersion == (WORD) -1)
    {
        FF_DEBUG("WinSock initialization previously failed");
        return "initWsaData() failed before";
    }

    struct addrinfo* addr;
    struct addrinfo hints = {
        .ai_family = state->ipv6 ? AF_INET6 : AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_NUMERICSERV,
    };

    FF_DEBUG("Resolving address: %s (%s)", host, state->ipv6 ? "IPv6" : "IPv4");
    if(getaddrinfo(host, "80", &hints, &addr) != 0)
    {
        FF_DEBUG("getaddrinfo() failed");
        return "getaddrinfo() failed";
    }

    state->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(state->sockfd == INVALID_SOCKET)
    {
        FF_DEBUG("socket() failed");
        freeaddrinfo(addr);
        return "socket() failed";
    }

    DWORD flag = 1;
    #ifdef TCP_NODELAY
    // Enable TCP_NODELAY to disable Nagle's algorithm
    if (setsockopt(state->sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) != 0) {
        FF_DEBUG("Failed to set TCP_NODELAY: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
    } else {
        FF_DEBUG("Successfully disabled Nagle's algorithm");
    }
    #endif

    // Set timeout if needed
    if (state->timeout > 0) {
        FF_DEBUG("Setting connection timeout: %u ms", state->timeout);
        setsockopt(state->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&state->timeout, sizeof(state->timeout));
    }

    //ConnectEx requires the socket to be initially bound
    if((state->ipv6
        ? bind(state->sockfd, (SOCKADDR *) &(struct sockaddr_in6) {
            .sin6_family = AF_INET6,
            .sin6_addr = in6addr_any,
        }, sizeof(struct sockaddr_in6))
        : bind(state->sockfd, (SOCKADDR *) &(struct sockaddr_in) {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
        }, sizeof(struct sockaddr_in))) != 0)
    {
        FF_DEBUG("bind() failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        closesocket(state->sockfd);
        freeaddrinfo(addr);
        state->sockfd = INVALID_SOCKET;
        return "bind() failed";
    }

    // Initialize overlapped structure for asynchronous I/O
    memset(&state->overlapped, 0, sizeof(OVERLAPPED));

    // Build HTTP command
    FF_STRBUF_AUTO_DESTROY command = ffStrbufCreateA(64);
    ffStrbufAppendS(&command, "GET ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, " HTTP/1.0\nHost: ");
    ffStrbufAppendS(&command, host);
    ffStrbufAppendS(&command, "\r\n");
    ffStrbufAppendS(&command, "Connection: close\r\n"); // Explicitly request connection closure

    // Add compression support if enabled
    if (state->compression) {
        FF_DEBUG("Enabling HTTP content compression");
        ffStrbufAppendS(&command, "Accept-Encoding: gzip\r\n");
    }

    ffStrbufAppendS(&command, headers);
    ffStrbufAppendS(&command, "\r\n");

    #ifdef TCP_FASTOPEN
    if (state->tfo)
    {
        // Set TCP Fast Open
        flag = 1;
        if (setsockopt(state->sockfd, IPPROTO_TCP, TCP_FASTOPEN, (char*)&flag, sizeof(flag)) != 0) {
            FF_DEBUG("Failed to set TCP_FASTOPEN option: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        } else {
            FF_DEBUG("Successfully set TCP_FASTOPEN option");
        }
    }
    else
    {
        FF_DEBUG("TCP Fast Open disabled");
    }
    #endif

    FF_DEBUG("Using ConnectEx to send %u bytes of data", command.length);
    DWORD sent = 0;
    BOOL result = ConnectEx(state->sockfd, addr->ai_addr, (int)addr->ai_addrlen,
                          command.chars, command.length, &sent, &state->overlapped);

    freeaddrinfo(addr);
    addr = NULL;

    if(!result)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            FF_DEBUG("ConnectEx() failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
            closesocket(state->sockfd);
            state->sockfd = INVALID_SOCKET;
            return "ConnectEx() failed";
        }
        else
        {
            FF_DEBUG("ConnectEx() pending");
        }
    }
    else
    {
        FF_DEBUG("ConnectEx() succeeded, sent %u bytes of data", (unsigned) sent);
    }

    // No need to cleanup state fields here since we need them in the receive function
    return NULL;
}

const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer)
{
    FF_DEBUG("Preparing to receive HTTP response");

    if (state->sockfd == INVALID_SOCKET)
    {
        FF_DEBUG("Invalid socket, HTTP request might have failed");
        return "ffNetworkingSendHttpRequest() failed";
    }

    uint32_t timeout = state->timeout;
    if (timeout > 0)
    {
        FF_DEBUG("WaitForSingleObject with timeout: %u ms", timeout);
        if (WaitForSingleObject((HANDLE)state->sockfd, timeout) != WAIT_OBJECT_0)
        {
            FF_DEBUG("WaitForSingleObject failed or timed out");
            CancelIo((HANDLE) state->sockfd);
            closesocket(state->sockfd);
            return "WaitForSingleObject(state->sockfd) failed or timeout";
        }
    }

    DWORD transfer, flags;
    if (!WSAGetOverlappedResult(state->sockfd, &state->overlapped, &transfer, TRUE, &flags))
    {
        FF_DEBUG("WSAGetOverlappedResult failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
        closesocket(state->sockfd);
        return "WSAGetOverlappedResult() failed";
    }

    if(timeout > 0)
    {
        FF_DEBUG("Setting receive timeout: %u ms", timeout);
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    }

    // Set larger receive buffer for better performance
    int rcvbuf = 65536; // 64KB
    setsockopt(state->sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvbuf, sizeof(rcvbuf));

    FF_DEBUG("Starting data reception");
    FF_MAYBE_UNUSED int recvCount = 0;
    uint32_t contentLength = 0;
    char* headerEnd = NULL;

    do {
        FF_DEBUG("Data reception loop #%d, current buffer size: %u, available space: %u",
                 ++recvCount, buffer->length, ffStrbufGetFree(buffer));

        ssize_t received = recv(state->sockfd, buffer->chars + buffer->length, (int)ffStrbufGetFree(buffer), 0);

        if (received <= 0) {
            if (received == 0) {
                FF_DEBUG("Connection closed (received=0)");
            } else {
                FF_DEBUG("Reception failed: %s", ffDebugWin32Error((DWORD) WSAGetLastError()));
            }
            break;
        }

        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';

        FF_DEBUG("Successfully received %zd bytes of data, total: %u bytes", received, buffer->length);

        // Check if HTTP header end marker is found
        if (headerEnd == NULL) {
            headerEnd = strstr(buffer->chars, "\r\n\r\n");
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
                        FF_DEBUG("Extended receive buffer to %u bytes", buffer->allocated);
                    }
                }
            }
        }
    } while (ffStrbufGetFree(buffer) > 0);

    FF_DEBUG("Closing socket: fd=%u", (unsigned)state->sockfd);
    closesocket(state->sockfd);
    state->sockfd = INVALID_SOCKET;

    if (buffer->length == 0) {
        FF_DEBUG("Server response is empty");
        return "Empty server response received";
    }

    if (headerEnd == NULL) {
        FF_DEBUG("No HTTP header end marker found");
        return "No HTTP header end found";
    }

    if (ffStrbufStartsWithS(buffer, "HTTP/1.0 200 OK\r\n")) {
        FF_DEBUG("Received valid HTTP 200 response, content length: %u bytes, total length: %u bytes",
                contentLength, buffer->length);
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
