#pragma once

#include "fastfetch.h"

#include <sys/types.h>
#include <sys/sysctl.h>

#ifdef __OpenBSD__
const char* ffSysctlGetString(int mib1, int mib2, FFstrbuf* result);
[[nodiscard]] int ffSysctlGetInt(int mib1, int mib2, int defaultValue);
[[nodiscard]] int64_t ffSysctlGetInt64(int mib1, int mib2, int64_t defaultValue);
#else
const char* ffSysctlGetString(const char* propName, FFstrbuf* result);
[[nodiscard]] int ffSysctlGetInt(const char* propName, int defaultValue);
[[nodiscard]] int64_t ffSysctlGetInt64(const char* propName, int64_t defaultValue);
#endif
[[nodiscard]] void* ffSysctlGetData(int* request, u_int requestLength, size_t* resultLength);
