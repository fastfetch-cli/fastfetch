#include "memory.h"

#include <string.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <unistd.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_MEMSIZE }, 2, &ram->bytesTotal, &length, NULL, 0))
        return "Failed to read hw.memsize";

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    if(host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t) (&vmstat), &count) != KERN_SUCCESS)
        return "Failed to read host_statistics64";

    // https://github.com/apple-opensource/top/blob/e7979606cf63270663a62cfe69f82d35cef9ba58/globalstats.c#L433-L435
    ram->bytesUsed = ((uint64_t)
        + vmstat.wire_count 
        + vmstat.inactive_count 
        + vmstat.active_count 
        + vmstat.compressor_page_count
    ) * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
