#pragma once

#ifndef FF_INCLUDED_common_sysctl
#define FF_INCLUDED_common_sysctl

#include "fastfetch.h"
#include "util/FFcheckmacros.h"

#include <sys/types.h>
#include <sys/sysctl.h>

const char* ffSysctlGetString(const char* propName, FFstrbuf* result);
FF_C_NODISCARD int ffSysctlGetInt(const char* propName, int defaultValue);
FF_C_NODISCARD int64_t ffSysctlGetInt64(const char* propName, int64_t defaultValue);
FF_C_NODISCARD void* ffSysctlGetData(int* request, u_int requestLength, size_t* resultLength);

#endif
