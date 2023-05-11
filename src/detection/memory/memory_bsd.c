#include "memory.h"
#include "common/sysctl.h"

const char* ffDetectMemory(FFMemoryResult* ram)
{
    uint32_t pageSize;
    uint64_t length = sizeof(pageSize);
    if (sysctl((int[]){ CTL_HW, HW_PAGESIZE }, 2, &pageSize, &length, NULL, 0))
        return "Failed to read hw.pagesize";

    ram->bytesTotal = (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_page_count", 0) * pageSize;
    if(ram->bytesTotal == 0)
        return "Failed to read vm.stats.vm.v_page_count";

    ram->bytesUsed = ram->bytesTotal
        - (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_free_count", 0) * pageSize
        - (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_inactive_count", 0) * pageSize
    ;

    return NULL;
}
