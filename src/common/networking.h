#pragma once

#ifndef FF_INCLUDED_common_networking
#define FF_INCLUDED_common_networking

#include "util/FFstrbuf.h"

void ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, FFstrbuf* buffer);

#endif
