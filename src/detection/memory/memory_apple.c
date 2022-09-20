#include "memory.h"
#include "common/sysctl.h"

#include <string.h>
#include <mach/mach.h>

static void detectRam(FFMemoryStorage* ram)
{
    ram->bytesTotal = (uint64_t) ffSysctlGetInt64("hw.memsize", 0);
    if(ram->bytesTotal == 0)
    {
        ffStrbufAppendS(&ram->error, "Failed to read hw.memsize");
        return;
    }

    uint32_t pagesize = (uint32_t) ffSysctlGetInt("hw.pagesize", 0);
    if(pagesize == 0)
    {
        ffStrbufAppendS(&ram->error, "Failed to read hw.pagesize");
        return;
    }

    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if(host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t) (&vmstat), &count) != KERN_SUCCESS)
    {
        ffStrbufAppendS(&ram->error, "Failed to read vm statistics");
        return;
    }

    ram->bytesUsed = ((uint64_t) vmstat.active_count + vmstat.wire_count) * pagesize;
}

static void detectSwap(FFMemoryStorage* swap)
{
    struct xsw_usage xsw;
    size_t size = sizeof(xsw);
    if(sysctlbyname("vm.swapusage", &xsw, &size, 0, 0) != 0)
    {
        ffStrbufAppendS(&swap->error, "Failed to read vm.swapusage");
        return;
    }

    swap->bytesTotal = xsw.xsu_total;
    swap->bytesUsed = xsw.xsu_used;
}

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    detectRam(&memory->ram);
    detectSwap(&memory->swap);
}
