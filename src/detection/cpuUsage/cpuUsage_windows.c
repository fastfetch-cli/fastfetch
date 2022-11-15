#include "fastfetch.h"
#include "cpuUsage.h"

#include <processthreadsapi.h>

static inline uint64_t fileTimeToUint64(const FILETIME* ft) {
    return (((uint64_t)ft->dwHighDateTime) << 32) | ((uint64_t)ft->dwLowDateTime);
}

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
{
    FILETIME idleTime, kernelTime, userTime;
    if(!GetSystemTimes(&idleTime, &kernelTime, &userTime))
        return "GetSystemTimes() failed";

    // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getsystemtimes
    // `kernelTime` also includes the amount of time the system has been idle.
    *totalAll = fileTimeToUint64(&userTime) + fileTimeToUint64(&kernelTime);
    *inUseAll = *totalAll - fileTimeToUint64(&idleTime);
    return NULL;
}
