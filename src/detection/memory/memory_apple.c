#include "memory.h"
#include "common/debug.h"

#include <string.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <unistd.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);

    #if FF_APPLE_MEMSIZE_USABLE
    if (sysctlbyname("hw.memsize_usable", &ram->bytesTotal, &length, NULL, 0) != 0)
        return "Failed to read hw.memsize_usable";
    #else
    if (sysctl((int[]){ CTL_HW, HW_MEMSIZE }, 2, &ram->bytesTotal, &length, NULL, 0) != 0)
        return "Failed to read hw.memsize";
    #endif

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    if(host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t) (&vmstat), &count) != KERN_SUCCESS)
        return "Failed to read host_statistics64";

    // https://github.com/st3fan/osx-10.9/blob/34e34a6a539b5a822cda4074e56a7ced9b57da71/system_cmds-597.1.1/vm_stat.tproj/vm_stat.c#L139

    uint64_t pagesFree = vmstat.free_count - vmstat.speculative_count;
    uint64_t pagesFileBacked = vmstat.external_page_count; // Cached files
    ram->bytesUsed = ram->bytesTotal - (pagesFree + pagesFileBacked) * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
