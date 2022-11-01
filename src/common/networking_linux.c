#include "fastfetch.h"
#include "common/networking.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

bool ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* addr;

    if(getaddrinfo(host, "80", &hints, &addr) != 0)
        return false;

    state->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(state->sockfd == -1)
    {
        freeaddrinfo(addr);
        return false;
    }

    if(connect(state->sockfd, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        close(state->sockfd);
        freeaddrinfo(addr);
        return false;
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

    if(send(state->sockfd, command.chars, command.length, 0) < 0)
    {
        ffStrbufDestroy(&command);
        close(state->sockfd);
        return false;
    }
    ffStrbufDestroy(&command);
    return true;
}

bool ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer, uint32_t timeout)
{
    if(timeout > 0)
    {
        struct timeval timev;
        timev.tv_sec = 0;
        timev.tv_usec = (__typeof__(timev.tv_usec)) (timeout * 1000); //milliseconds to microseconds
        setsockopt(state->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    }

    ssize_t received = recv(state->sockfd, buffer->chars + buffer->length, ffStrbufGetFree(buffer), 0);

    if(received > 0)
    {
        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';
    }

    close(state->sockfd);
    return ffStrbufStartsWithS(buffer, "HTTP/1.1 200 OK\r\n");
}
