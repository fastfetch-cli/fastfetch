#include "memory.h"
#include "common/sysctl.h"

static void detectRam(FFMemoryStorage* ram, FFMemoryResult* memory)
{
    uint32_t pageSize = (uint32_t) ffSysctlGetInt("hw.pagesize", 0);
    if (pageSize == 0)
    {
        ffStrbufAppendS(&ram->error, "Failed to read hw.pagesize");
        return;
    }

    memory->ram.bytesTotal = (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_page_count", 0) * pageSize;
    if (ram->bytesTotal == 0)
    {
        ffStrbufAppendS(&ram->error, "Failed to read vm.stats.vm.v_page_count");
        return;
    }

    memory->ram.bytesUsed = memory->ram.bytesTotal
        - (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_free_count", 0) * pageSize
        - (uint64_t) ffSysctlGetInt64("vm.stats.vm.v_inactive_count", 0) * pageSize
    ;
}

static void detectSwap(FFMemoryStorage* swap)
{
    ffStrbufAppendS(&swap->error, "Not implemented");
}

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    detectRam(&memory->ram, memory);
    detectSwap(&memory->swap);
}
