#pragma once

#ifndef FF_INCLUDED_common_networking
#define FF_INCLUDED_common_networking

#include "util/FFstrbuf.h"

int ffNetworkingSendHttpRequest(const char* host, const char* path, const char* headers, uint32_t timeout);
void ffNetworkingRecvHttpResponse(int sock, FFstrbuf* buffer);
void ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, const char* headers, FFstrbuf* buffer);

#endif
