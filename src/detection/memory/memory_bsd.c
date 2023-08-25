#include "memory.h"
#include "common/sysctl.h"

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_PHYSMEM }, 2, &ram->bytesTotal, &length, NULL, 0))
        return "Failed to read hw.physmem";

    uint32_t pageSize;
    length = sizeof(pageSize);
    if (sysctl((int[]){ CTL_HW, HW_PAGESIZE }, 2, &pageSize, &length, NULL, 0))
        return "Failed to read hw.pagesize";

    // vm.stats.vm.* are int values
    int32_t pagesFree = ffSysctlGetInt("vm.stats.vm.v_free_count", 0)
        + ffSysctlGetInt("vm.stats.vm.v_inactive_count", 0)
        + ffSysctlGetInt("vm.stats.vm.v_cache_count", 0);

    ram->bytesUsed = ram->bytesTotal - (uint64_t) pagesFree * pageSize;

    return NULL;
}
