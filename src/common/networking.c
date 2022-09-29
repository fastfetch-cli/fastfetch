#include "fastfetch.h"
#include "common/networking.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

int ffNetworkingSendHttpRequest(const char* host, const char* path, const char* headers, uint32_t timeout)
{
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* addr;

    if(getaddrinfo(host, "80", &hints, &addr) != 0)
        return -1;

    int sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(sockfd == -1)
    {
        freeaddrinfo(addr);
        return -1;
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
        close(sockfd);
        freeaddrinfo(addr);
        return -1;
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
        close(sockfd);
        return -1;
    }
    ffStrbufDestroy(&command);
    return sockfd;
}

void ffNetworkingRecvHttpResponse(int sockfd, FFstrbuf* buffer)
{
    ssize_t received = recv(sockfd, buffer->chars + buffer->length, ffStrbufGetFree(buffer), 0);

    if(received > 0)
    {
        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';
    }

    close(sockfd);
}

void ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, const char* headers, FFstrbuf* buffer)
{
    int sockfd = ffNetworkingSendHttpRequest(host, path, headers, timeout);
    if(sockfd > 0)
        ffNetworkingRecvHttpResponse(sockfd, buffer);
}
