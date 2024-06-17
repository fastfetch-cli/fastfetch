#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

#include "util/mallocHelper.h"

#include <ntstatus.h>
#include <winternl.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessorPerformanceInformation, NULL, 0, &size) != STATUS_INFO_LENGTH_MISMATCH)
        return "NtQuerySystemInformation(SystemProcessorPerformanceInformation, NULL) failed";

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* FF_AUTO_FREE pinfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*)malloc(size);
    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessorPerformanceInformation, pinfo, size, &size)))
        return "NtQuerySystemInformation(SystemProcessorPerformanceInformation, size) failed";

    for (uint32_t i = 0; i < size / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION); ++i)
    {
        SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* coreInfo = pinfo + i;

        // KernelTime includes idle time.
        LONGLONG dpcTime = coreInfo->Reserved1[0].QuadPart;
        LONGLONG interruptTime = coreInfo->Reserved1[1].QuadPart;
        coreInfo->KernelTime.QuadPart -= coreInfo->IdleTime.QuadPart;
        coreInfo->KernelTime.QuadPart += dpcTime + interruptTime;

        uint64_t inUse = (uint64_t) (coreInfo->UserTime.QuadPart + coreInfo->KernelTime.QuadPart);
        uint64_t total = inUse + (uint64_t)coreInfo->IdleTime.QuadPart;

        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        *info = (FFCpuUsageInfo) {
            .inUseAll = inUse,
            .totalAll = total,
        };
    }

    return NULL;
}
