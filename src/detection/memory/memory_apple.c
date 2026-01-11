#include "memory.h"

#include <string.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <unistd.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_MEMSIZE }, 2, &ram->bytesTotal, &length, NULL, 0) != 0)
        return "Failed to read hw.memsize";
    uint64_t usableMemory = 0;
    length = sizeof(usableMemory);
    if (sysctlbyname("hw.memsize_usable", &usableMemory, &length, NULL, 0) != 0)
        usableMemory = ram->bytesTotal;

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    if(host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t) (&vmstat), &count) != KERN_SUCCESS)
        return "Failed to read host_statistics64";

    uint64_t pageSize = instance.state.platform.sysinfo.pageSize;
    uint64_t appMemory = vmstat.internal_page_count * pageSize;
    uint64_t wiredMemory = vmstat.wire_count * pageSize;
    uint64_t compressed = vmstat.compressor_page_count * pageSize;
    uint64_t reserved = ram->bytesTotal - usableMemory;

    ram->bytesUsed = appMemory + wiredMemory + compressed + reserved;

    return NULL;
}
