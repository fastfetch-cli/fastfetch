#include <mswsock.h>
#include <ws2tcpip.h>

//Must be included after <mswsock.h>
#include "fastfetch.h"
#include "common/networking.h"

static LPFN_CONNECTEX ConnectEx;

static const char* initWsaData(WSADATA* wsaData)
{
    if(WSAStartup(MAKEWORD(2, 2), wsaData) != 0)
        return "WSAStartup() failed";

    if(LOBYTE(wsaData->wVersion) != 2 || HIBYTE(wsaData->wVersion) != 2)
        return "Invalid wsaData version found";

    //Dummy socket needed for WSAIoctl
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == INVALID_SOCKET)
        return "socket(AF_INET, SOCK_STREAM) failed";

    DWORD dwBytes;
    GUID guid = WSAID_CONNECTEX;
    if(WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guid, sizeof(guid),
                    &ConnectEx, sizeof(ConnectEx),
                    &dwBytes, NULL, NULL) != 0)
        return "WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER) failed";

    closesocket(sockfd);

    return NULL;
}

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    static WSADATA wsaData;
    if (wsaData.wVersion == 0)
    {
        const char* error = initWsaData(&wsaData);
        if (error != NULL)
        {
            wsaData.wVersion = (WORD) -1;
            return error;
        }
    }
    else if (wsaData.wVersion == (WORD) -1)
        return "initWsaData() failed before";

    struct addrinfo* addr;

    if(getaddrinfo(host, "80", &(struct addrinfo) {
        .ai_family = state->ipv6 ? AF_INET6 : AF_INET,
        .ai_socktype = SOCK_STREAM,
    }, &addr) != 0)
        return "getaddrinfo() failed";

    state->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(state->sockfd == INVALID_SOCKET)
    {
        freeaddrinfo(addr);
        return "socket() failed";
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
        closesocket(state->sockfd);
        freeaddrinfo(addr);
        state->sockfd = INVALID_SOCKET;
        return "bind() failed";
    }

    FF_STRBUF_AUTO_DESTROY command = ffStrbufCreateA(64);
    ffStrbufAppendS(&command, "GET ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, " HTTP/1.1\nHost: ");
    ffStrbufAppendS(&command, host);
    ffStrbufAppendS(&command, "\r\n");
    ffStrbufAppendS(&command, headers);
    ffStrbufAppendS(&command, "\r\n");

    BOOL result = ConnectEx(state->sockfd, addr->ai_addr, (int)addr->ai_addrlen, command.chars, command.length, NULL, &state->overlapped);
    freeaddrinfo(addr);

    if(!result && WSAGetLastError() != WSA_IO_PENDING)
    {
        closesocket(state->sockfd);
        state->sockfd = INVALID_SOCKET;
        return "ConnectEx() failed";
    }

    return NULL;
}

const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer)
{
    if (state->sockfd == INVALID_SOCKET)
        return "ffNetworkingSendHttpRequest() failed";

    uint32_t timeout = state->timeout;
    if (timeout > 0)
    {
        if (WaitForSingleObject((HANDLE) state->sockfd, timeout) != WAIT_OBJECT_0)
        {
            CancelIo((HANDLE) state->sockfd);
            closesocket(state->sockfd);
            return "WaitForSingleObject(state->sockfd) failed or timeout";
        }
    }

    DWORD transfer, flags;
    if (!WSAGetOverlappedResult(state->sockfd, &state->overlapped, &transfer, TRUE, &flags))
    {
        closesocket(state->sockfd);
        return "WSAGetOverlappedResult() failed";
    }

    if(timeout > 0)
    {
        //https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-setsockopt
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));
    }

    uint32_t recvStart;
    do {
        recvStart = buffer->length;
        ssize_t received = recv(state->sockfd, buffer->chars + buffer->length, (int) ffStrbufGetFree(buffer), 0);
        if (received <= 0) break;
        buffer->length = recvStart + (uint32_t) received;
        buffer->chars[buffer->length] = '\0';
    } while (ffStrbufGetFree(buffer) > 0 && strstr(buffer->chars + recvStart, "\r\n\r\n") == NULL);

    closesocket(state->sockfd);
    if (buffer->length == 0) return "Empty server response received";
    return ffStrbufStartsWithS(buffer, "HTTP/1.1 200 OK\r\n") ? NULL : "Invalid response";
}
