#pragma once

#ifndef FF_INCLUDED_common_networking
#define FF_INCLUDED_common_networking

#include "util/FFstrbuf.h"

#ifdef _WIN32
    #include <minwindef.h>
#endif

typedef struct FFNetworkingState {
    #ifdef _WIN32
    uintptr_t sockfd;
    OVERLAPPED overlapped;
    #else
    int sockfd;
    #endif
} FFNetworkingState;

bool ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers);
bool ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer, uint32_t timeout);

static inline bool ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, const char* headers, FFstrbuf* buffer)
{
    FFNetworkingState state;
    if(ffNetworkingSendHttpRequest(&state, host, path, headers))
        return ffNetworkingRecvHttpResponse(&state, buffer, timeout);
    return false;
}

#endif
