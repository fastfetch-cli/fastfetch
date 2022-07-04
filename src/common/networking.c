#include "fastfetch.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

void ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, FFstrbuf* buffer)
{
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    struct addrinfo* addr;

    if(getaddrinfo(host, "80", &hints, &addr) != 0)
        return;

    int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(sock == -1)
    {
        freeaddrinfo(addr);
        return;
    }

    if(timeout > 0)
    {
        struct timeval timev;
        timev.tv_sec = 0;
        timev.tv_usec = timeout * 1000; //milliseconds to microseconds
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    }

    if(connect(sock, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        close(sock);
        freeaddrinfo(addr);
        return;
    }

    freeaddrinfo(addr);

    FFstrbuf command;
    ffStrbufInitA(&command, 64);
    ffStrbufAppendS(&command, "GET ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, " HTTP/1.1\nHost: ");
    ffStrbufAppendS(&command, host);
    ffStrbufAppendS(&command, "\r\n\r\n");

    if(send(sock, command.chars, command.length, 0) == -1)
    {
        ffStrbufDestroy(&command);
        close(sock);
        return;
    }

    ssize_t received = recv(sock, buffer->chars + buffer->length, ffStrbufGetFree(buffer), 0);

    if(received > 0)
    {
        buffer->length += (uint32_t) received;
        buffer->chars[buffer->length] = '\0';
    }

    ffStrbufDestroy(&command);
    close(sock);
}
