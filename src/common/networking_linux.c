#include "fastfetch.h"
#include "common/networking.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

static void connectAndSend(FFNetworkingState* state)
{
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* addr;

    if(getaddrinfo(state->host.chars, "80", &hints, &addr) != 0)
        goto error;

    state->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(state->sockfd == -1)
    {
        freeaddrinfo(addr);
        goto error;
    }

    if(connect(state->sockfd, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        close(state->sockfd);
        freeaddrinfo(addr);
        goto error;
    }

    freeaddrinfo(addr);

    if(send(state->sockfd, state->command.chars, state->command.length, 0) < 0)
    {
        close(state->sockfd);
        goto error;
    }

    goto exit;

error:
    state->sockfd = -1;

exit:
    ffStrbufDestroy(&state->host);
    ffStrbufDestroy(&state->command);
}

FF_THREAD_ENTRY_DECL_WRAPPER(connectAndSend, FFNetworkingState*);

bool ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    ffStrbufInitS(&state->host, host);

    ffStrbufInitA(&state->command, 64);
    ffStrbufAppendS(&state->command, "GET ");
    ffStrbufAppendS(&state->command, path);
    ffStrbufAppendS(&state->command, " HTTP/1.1\nHost: ");
    ffStrbufAppendS(&state->command, host);
    ffStrbufAppendS(&state->command, "\r\n");
    ffStrbufAppendS(&state->command, headers);
    ffStrbufAppendS(&state->command, "\r\n");

    #ifdef FF_HAVE_THREADS
        state->thread = ffThreadCreate(connectAndSendThreadMain, state);
        return !!state->thread;
    #else
        connectAndSend(state);
        return state->sockfd != -1;
    #endif
}

bool ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer, uint32_t timeout)
{
    #ifdef FF_HAVE_THREADS
        if (!ffThreadJoin(state->thread, timeout))
            return false;
    #endif
    if(state->sockfd == -1)
        return false;

    if(timeout > 0)
    {
        struct timeval timev;
        timev.tv_sec = timeout / 1000;
        timev.tv_usec = (__typeof__(timev.tv_usec)) ((timeout % 1000) * 1000); //milliseconds to microseconds
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
