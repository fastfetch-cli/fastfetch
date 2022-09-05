#include "memory.h"
#include "common/settings.h"

#include <mach/mach.h>

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    memory->bytesTotal = (uint64_t) ffSettingsGetAppleInt64("hw.memsize", 0);

    memory->bytesUsed = 0;

    uint32_t pagesize = (uint32_t) ffSettingsGetAppleInt("hw.pagesize", 0);
    if(pagesize == 0)
        return;

    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if(host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t) (&vmstat), &count) != KERN_SUCCESS)
        return;

    memory->bytesUsed = ((uint64_t) vmstat.active_count + vmstat.wire_count) * pagesize;
}
