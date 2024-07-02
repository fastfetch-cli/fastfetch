#include "memory.h"
#include "common/sysctl.h"

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_PHYSMEM }, 2, &ram->bytesTotal, &length, NULL, 0))
        return "Failed to read hw.physmem";

    // vm.stats.vm.* are int values
    int32_t pagesFree = ffSysctlGetInt("vm.stats.vm.v_free_count", 0)
        + ffSysctlGetInt("vm.stats.vm.v_inactive_count", 0)
        + ffSysctlGetInt("vm.stats.vm.v_cache_count", 0);

    ram->bytesUsed = ram->bytesTotal - (uint64_t) pagesFree * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
