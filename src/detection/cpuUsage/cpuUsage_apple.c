#include "fastfetch.h"
#include "cpuUsage.h"

#include <mach/processor_info.h>
#include <mach/mach_host.h>
#include <unistd.h>

static const char* getCpuUsageInfo(long* inUseAll, long* totalAll)
{
    natural_t numCPUs = 0U;
    processor_info_array_t cpuInfo;
    mach_msg_type_number_t numCpuInfo;

    if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numCPUs, &cpuInfo, &numCpuInfo) != KERN_SUCCESS)
        return "host_processor_info() failed";
    if (numCPUs * CPU_STATE_MAX != numCpuInfo)
        return "Unexpected host_processor_info() result";

    for (natural_t i = 0U; i < numCPUs; ++i) {
        integer_t inUse = cpuInfo[CPU_STATE_MAX * i + CPU_STATE_USER]
            + cpuInfo[CPU_STATE_MAX * i + CPU_STATE_SYSTEM]
            + cpuInfo[CPU_STATE_MAX * i + CPU_STATE_NICE];
        integer_t total = inUse + cpuInfo[CPU_STATE_MAX * i + CPU_STATE_IDLE];
        *inUseAll += inUse;
        *totalAll += total;
    }
    return NULL;
}

const char* ffGetCpuUsagePercent(double* result)
{
    long inUseAll1 = 0, totalAll1 = 0;
    const char* error = getCpuUsageInfo(&inUseAll1, &totalAll1);
    if(error)
        return error;

    sleep(1);

    long inUseAll2 = 0, totalAll2 = 0;
    error = getCpuUsageInfo(&inUseAll2, &totalAll2);
    if(error)
        return error;

    *result = (double)(inUseAll2 - inUseAll1) / (double)(totalAll2 - totalAll1) * 100;
    return NULL;
}
