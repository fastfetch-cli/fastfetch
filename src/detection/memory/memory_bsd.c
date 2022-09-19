#include "memory.h"

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    memory->ram.bytesTotal = 0;
    memory->ram.bytesUsed = 0;
    memory->swap.bytesTotal = 0;
    memory->swap.bytesUsed = 0;
}
