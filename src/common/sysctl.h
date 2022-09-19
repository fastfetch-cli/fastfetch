#pragma once

#ifndef FF_INCLUDED_common_sysctl
#define FF_INCLUDED_common_sysctl

#include "fastfetch.h"

void ffSysctlGetString(const char* propName, FFstrbuf* result);
int ffSysctlGetInt(const char* propName, int defaultValue);
int64_t ffSysctlGetInt64(const char* propName, int64_t defaultValue);
void* ffSysctlGetData(int* request, int requestLength, size_t* resultLength);

#endif
