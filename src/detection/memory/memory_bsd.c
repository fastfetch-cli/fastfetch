#include "memory.h"
#include "common/sysctl.h"

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    memory->ram.bytesTotal = (uint64_t) ffSysctlGetInt64("hw.physmem", 0);
    memory->ram.bytesUsed = 0;

    memory->swap.bytesTotal = 0;
    memory->swap.bytesUsed = 0;
}
