#pragma once

#include "fastfetch.h"

#include <sys/types.h>
#include <sys/sysctl.h>

// `result` is always written to, and `propName` is always read by the sysctl wrapper.
#ifdef __OpenBSD__
[[gnu::nonnull(3)]] const char* ffSysctlGetString(int mib1, int mib2, FFstrbuf* result);
[[nodiscard]] int ffSysctlGetInt(int mib1, int mib2, int defaultValue);
[[nodiscard]] int64_t ffSysctlGetInt64(int mib1, int mib2, int64_t defaultValue);
#else
[[gnu::nonnull(1, 2)]] const char* ffSysctlGetString(const char* propName, FFstrbuf* result);
[[gnu::nonnull(1), nodiscard]] int ffSysctlGetInt(const char* propName, int defaultValue);
[[gnu::nonnull(1), nodiscard]] int64_t ffSysctlGetInt64(const char* propName, int64_t defaultValue);
#endif
