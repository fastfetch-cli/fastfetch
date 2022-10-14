#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <synchapi.h>

    static BOOL WINAPI initWsaData(PINIT_ONCE once, PVOID param, PVOID* context)
    {
        (void)once;
        (void)param;
        static WSADATA wsaData;
        *context = &wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }

    //Types of winsock2 are full of mess. Disable warnings for them and keep clean for posix
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
    #pragma GCC diagnostic ignored "-Wconversion"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#else
    #include <unistd.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <netdb.h>

    #define closesocket close
#endif

//Must be included after <winsock2.h>
#include "fastfetch.h"
#include "common/networking.h"

FFSockType ffNetworkingSendHttpRequest(const char* host, const char* path, const char* headers, uint32_t timeout)
{
    #ifdef _WIN32
    static INIT_ONCE once = INIT_ONCE_STATIC_INIT;
    WSADATA* pData;
    if(!InitOnceExecuteOnce(&once, initWsaData, NULL, (LPVOID*) &pData))
        return INVALID_SOCKET;
    #endif

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* addr;

    if(getaddrinfo(host, "80", &hints, &addr) != 0)
        return INVALID_SOCKET;

    FFSockType sockfd = (FFSockType)socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(sockfd == INVALID_SOCKET)
    {
        freeaddrinfo(addr);
        return INVALID_SOCKET;
    }

    if(timeout > 0)
    {
        struct timeval timev;
        timev.tv_sec = 0;
        timev.tv_usec = (__typeof__(timev.tv_usec)) (timeout * 1000); //milliseconds to microseconds
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    }

    if(connect(sockfd, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        closesocket(sockfd);
        freeaddrinfo(addr);
        return INVALID_SOCKET;
    }

    freeaddrinfo(addr);

    FFstrbuf command;
    ffStrbufInitA(&command, 64);
    ffStrbufAppendS(&command, "GET ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, " HTTP/1.1\nHost: ");
    ffStrbufAppendS(&command, host);
    ffStrbufAppendS(&command, "\r\n");
    ffStrbufAppendS(&command, headers);
    ffStrbufAppendS(&command, "\r\n");

    if(send(sockfd, command.chars, command.length, 0) == -1)
    {
        ffStrbufDestroy(&command);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }
    ffStrbufDestroy(&command);
    return sockfd;
}

bool ffNetworkingRecvHttpResponse(FFSockType sockfd, FFstrbuf* buffer)
{
    ssize_t received = recv(sockfd, buffer->chars + buffer->length, ffStrbufGetFree(buffer), 0);

    if(received > 0)
    {
        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';
    }

    closesocket(sockfd);
    return ffStrbufStartsWithS(buffer, "HTTP/1.1 200 OK\r\n");
}

bool ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, const char* headers, FFstrbuf* buffer)
{
    FFSockType sockfd = ffNetworkingSendHttpRequest(host, path, headers, timeout);
    if(sockfd != INVALID_SOCKET)
        return ffNetworkingRecvHttpResponse(sockfd, buffer);
    return false;
}
