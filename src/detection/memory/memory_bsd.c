#include "memory.h"
#include "common/sysctl.h"

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    uint32_t pageSize = (uint32_t) ffSysctlGetInt("hw.pagesize", 0);

    memory->ram.bytesTotal = (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_page_count", 0) * pageSize;
    
    memory->ram.bytesUsed = memory->ram.bytesTotal
        - (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_free_count", 0) * pageSize
        - (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_inactive_count", 0) * pageSize
    ;

    memory->swap.bytesTotal = 0;
    memory->swap.bytesUsed = 0;
}
