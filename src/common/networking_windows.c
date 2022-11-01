#include <mswsock.h>
#include <ws2tcpip.h>

//Must be included after <mswsock.h>
#include "fastfetch.h"
#include "common/networking.h"

static LPFN_CONNECTEX ConnectEx;

static BOOL WINAPI initWsaData(PINIT_ONCE once, PVOID param, PVOID* context)
{
    (void)once;
    (void)param;
    static WSADATA wsaData;
    *context = &wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return FALSE;

    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        return FALSE;

    //Dummy socket needed for WSAIoctl
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == INVALID_SOCKET)
        return FALSE;

    DWORD dwBytes;
    GUID guid = WSAID_CONNECTEX;
    if(WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guid, sizeof(guid),
                    &ConnectEx, sizeof(ConnectEx),
                    &dwBytes, NULL, NULL) != 0)
        return FALSE;

    return closesocket(sockfd) == 0;
}

bool ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    static INIT_ONCE once = INIT_ONCE_STATIC_INIT;
    WSADATA* pData;
    if(!InitOnceExecuteOnce(&once, initWsaData, NULL, (LPVOID*) &pData))
        return false;

    memset(state, 0, sizeof(*state));

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* addr;

    if(getaddrinfo(host, "80", &hints, &addr) != 0)
        return false;

    state->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(state->sockfd == INVALID_SOCKET)
    {
        freeaddrinfo(addr);
        return false;
    }

    {
        //ConnectEx requires the socket to be initially bound
        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = 0,
        };
        if(bind(state->sockfd, (SOCKADDR *)&addr, sizeof(addr)) != 0)
        {
            printf("bind %d\n", WSAGetLastError());
            return false;
        }
    }

    FFstrbuf command;
    ffStrbufInitA(&command, 64);
    ffStrbufAppendS(&command, "GET ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, " HTTP/1.1\nHost: ");
    ffStrbufAppendS(&command, host);
    ffStrbufAppendS(&command, "\r\n");
    ffStrbufAppendS(&command, headers);
    ffStrbufAppendS(&command, "\r\n");

    BOOL result = ConnectEx(state->sockfd, addr->ai_addr, (int)addr->ai_addrlen, command.chars, command.length, NULL, &state->overlapped);
    freeaddrinfo(addr);
    ffStrbufDestroy(&command);

    if(!result && WSAGetLastError() != WSA_IO_PENDING)
    {
        closesocket(state->sockfd);
        return false;
    }

    ffStrbufDestroy(&command);
    return true;
}

bool ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer, uint32_t timeout)
{
    DWORD transfer, flags;
    if (!WSAGetOverlappedResult(state->sockfd, &state->overlapped, &transfer, TRUE, &flags))
    {
        closesocket(state->sockfd);
        return false;
    }

    if(timeout > 0)
    {
        //https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-setsockopt
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    }

    ssize_t received = recv(state->sockfd, buffer->chars + buffer->length, (int)ffStrbufGetFree(buffer), 0);

    if(received > 0)
    {
        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';
    }

    closesocket(state->sockfd);
    return ffStrbufStartsWithS(buffer, "HTTP/1.1 200 OK\r\n");
}
