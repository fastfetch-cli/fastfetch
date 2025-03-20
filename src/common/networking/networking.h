#pragma once

#include "common/thread.h"
#include "util/FFstrbuf.h"

#ifdef _WIN32
    #include <minwindef.h>
#endif

struct addrinfo;

typedef struct FFNetworkingState {
    #ifdef _WIN32
        uintptr_t sockfd;
        OVERLAPPED overlapped;
    #else
        int sockfd;
        FFstrbuf command;
        struct addrinfo* addr;

        #ifdef FF_HAVE_THREADS
            FFThreadType thread;
        #endif
    #endif

    uint32_t timeout;
    bool ipv6;
    bool compression; // if true, HTTP content compression will be enabled if supported
    bool tfo; // if true, TCP Fast Open will be attempted first, and fallback to traditional connection if it fails
} FFNetworkingState;

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers);
const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer);

#ifdef FF_HAVE_ZLIB
const char* ffNetworkingLoadZlibLibrary(void);
bool ffNetworkingDecompressGzip(FFstrbuf* buffer, char* headerEnd);
#endif
