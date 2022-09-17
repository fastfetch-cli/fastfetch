#include "memory.h"
#include "common/settings.h"

#include <string.h>
#include <mach/mach.h>
#include <sys/sysctl.h>

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    memset(memory, 0, sizeof(FFMemoryResult));

    memory->ram.bytesTotal = (uint64_t) ffSettingsGetAppleInt64("hw.memsize", 0);

    uint32_t pagesize = (uint32_t) ffSettingsGetAppleInt("hw.pagesize", 0);
    if(pagesize == 0)
        return;

    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if(host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t) (&vmstat), &count) != KERN_SUCCESS)
        return;

    memory->ram.bytesUsed = ((uint64_t) vmstat.active_count + vmstat.wire_count) * pagesize;

    struct xsw_usage swap;
    size_t size = sizeof(swap);
    if (sysctlbyname("vm.swapusage", &swap, &size, 0, 0) == -1)
        return;
    memory->swap.bytesTotal = swap.xsu_total;
    memory->swap.bytesUsed = swap.xsu_used;
}
