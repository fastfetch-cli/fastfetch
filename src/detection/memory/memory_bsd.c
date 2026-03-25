#include "memory.h"
#include "common/sysctl.h"

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_PHYSMEM }, 2, &ram->bytesTotal, &length, NULL, 0))
        return "Failed to read hw.physmem";

    // calculates the reclaimable ARC pages.
    int64_t arcSize = ffSysctlGetInt64("kstat.zfs.misc.arcstats.size", 0);
    int64_t arcMin = ffSysctlGetInt64("vfs.zfs.arc_min", 0);
    int64_t arcPages = (arcSize > arcMin) ? (arcSize - arcMin)
        / instance.state.platform.sysinfo.pageSize : 0;

    // vm.stats.vm.* are int values
    int32_t pagesFree = ffSysctlGetInt("vm.stats.vm.v_free_count", 0)
        + ffSysctlGetInt("vm.stats.vm.v_inactive_count", 0)
        + ffSysctlGetInt("vm.stats.vm.v_cache_count", 0)
        + (int32_t) arcPages;

    ram->bytesUsed = ram->bytesTotal - (uint64_t) pagesFree * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
