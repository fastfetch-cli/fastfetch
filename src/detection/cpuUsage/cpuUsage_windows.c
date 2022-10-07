#include "fastfetch.h"
#include "cpuUsage.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static inline uint64_t fileTimeToUint64(const FILETIME* ft) {
    return (((uint64_t)ft->dwHighDateTime) << 32) | ((uint64_t)ft->dwLowDateTime);
}

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
{
    FILETIME idleTime, kernelTime, userTime;
    if(GetSystemTimes(&idleTime, &kernelTime, &userTime) == 0)
        return "GetSystemTimes() failed";

    *inUseAll = fileTimeToUint64(&userTime) + fileTimeToUint64(&kernelTime);
    *totalAll = *inUseAll + fileTimeToUint64(&idleTime);
    return NULL;
}
