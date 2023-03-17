#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

#include <mach/processor_info.h>
#include <mach/mach_host.h>

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
{
	host_cpu_load_info_data_t cpustats;
	mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    *inUseAll = *totalAll = 0;

    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)(&cpustats), &count) != KERN_SUCCESS)
        return "host_statistics() failed";

    *inUseAll = cpustats.cpu_ticks[CPU_STATE_USER]
        + cpustats.cpu_ticks[CPU_STATE_SYSTEM]
        + cpustats.cpu_ticks[CPU_STATE_NICE];
    *totalAll = *inUseAll + cpustats.cpu_ticks[CPU_STATE_IDLE];

    return NULL;
}
