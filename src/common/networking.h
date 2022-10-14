#pragma once

#ifndef FF_INCLUDED_common_networking
#define FF_INCLUDED_common_networking

#include "util/FFstrbuf.h"

#ifdef _WIN32
    typedef uintptr_t FFSockType; //SOCKET, unsigned
    #ifndef INVALID_SOCKET //Don't conflict with <winsock2.h>
        #define INVALID_SOCKET ((uintptr_t)~0)
    #endif
#else
    typedef int FFSockType; // signed
    #define INVALID_SOCKET (-1)
#endif

FFSockType ffNetworkingSendHttpRequest(const char* host, const char* path, const char* headers, uint32_t timeout);
bool ffNetworkingRecvHttpResponse(FFSockType sockfd, FFstrbuf* buffer);
bool ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, const char* headers, FFstrbuf* buffer);

#endif
