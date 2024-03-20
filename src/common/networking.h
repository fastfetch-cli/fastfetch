#pragma once

#include "common/thread.h"
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
        FFstrbuf host;
        FFstrbuf command;

        #ifdef FF_HAVE_THREADS
            FFThreadType thread;
        #endif
    #endif

    uint32_t timeout;
} FFNetworkingState;

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers);
const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer);
