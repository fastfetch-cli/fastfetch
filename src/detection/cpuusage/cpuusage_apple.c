#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

#include <mach/processor_info.h>
#include <mach/mach_host.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    natural_t numCPUs = 0U;
    processor_info_array_t cpuInfo;
    mach_msg_type_number_t numCpuInfo;

    if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numCPUs, &cpuInfo, &numCpuInfo) != KERN_SUCCESS)
        return "host_processor_info() failed";
    if (numCPUs * CPU_STATE_MAX != numCpuInfo)
        return "Unexpected host_processor_info() result";

    for (natural_t i = 0U; i < numCPUs; ++i)
    {
        integer_t inUse = cpuInfo[CPU_STATE_MAX * i + CPU_STATE_USER]
            + cpuInfo[CPU_STATE_MAX * i + CPU_STATE_SYSTEM]
            + cpuInfo[CPU_STATE_MAX * i + CPU_STATE_NICE];
        integer_t total = inUse + cpuInfo[CPU_STATE_MAX * i + CPU_STATE_IDLE];

        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        *info = (FFCpuUsageInfo) {
            .inUseAll = (uint64_t)inUse,
            .totalAll = (uint64_t)total,
        };
    }
    return NULL;
}
