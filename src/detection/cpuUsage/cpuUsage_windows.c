#include "fastfetch.h"
#include "cpuUsage.h"

#ifdef FF_USE_WIN_NTAPI

#include "util/mallocHelper.h"

#include <ntstatus.h>
#include <winternl.h>

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
{
    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessorPerformanceInformation, NULL, 0, &size) != STATUS_INFO_LENGTH_MISMATCH)
        return "NtQuerySystemInformation(SystemProcessorPerformanceInformation, NULL) failed";

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* FF_AUTO_FREE pinfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*)malloc(size);
    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessorPerformanceInformation, pinfo, size, &size)))
        return "NtQuerySystemInformation(SystemProcessorPerformanceInformation, size) failed";

    *inUseAll = *totalAll = 0;

    for (uint32_t i = 0; i < size / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION); ++i)
    {
        SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* coreInfo = pinfo + i;

        // KernelTime includes idle time.
        LONGLONG dpcTime = coreInfo->Reserved1[0].QuadPart;
        LONGLONG interruptTime = coreInfo->Reserved1[1].QuadPart;
        coreInfo->KernelTime.QuadPart -= coreInfo->IdleTime.QuadPart;
        coreInfo->KernelTime.QuadPart += dpcTime + interruptTime;

        LONGLONG inUse = coreInfo->UserTime.QuadPart + coreInfo->KernelTime.QuadPart;
        *inUseAll += (uint64_t)inUse;
        *totalAll += (uint64_t)(inUse + coreInfo->IdleTime.QuadPart);
    }

    return NULL;
}

#else

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

#endif
