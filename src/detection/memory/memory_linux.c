#include "memory.h"

#include <stdlib.h>
#include <string.h>

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
    {
        ffStrbufAppendS(&memory->ram.error, "Failed to open /proc/meminfo");
        ffStrbufAppendS(&memory->swap.error, "Failed to open /proc/meminfo");
        return;
    }

    char* line = NULL;
    size_t len = 0;

    uint32_t memTotal = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0,
             swapTotal = 0,
             swapFree = 0;

    while (getline(&line, &len, meminfo) != EOF)
    {
        sscanf(line, "MemTotal: %u", &memTotal);
        sscanf(line, "Shmem: %u", &shmem);
        sscanf(line, "MemFree: %u", &memFree);
        sscanf(line, "Buffers: %u", &buffers);
        sscanf(line, "Cached: %u", &cached);
        sscanf(line, "SReclaimable: %u", &sReclaimable);
        sscanf(line, "SwapTotal: %u", &swapTotal);
        sscanf(line, "SwapFree: %u", &swapFree);
    }

    if(line != NULL)
        free(line);

    fclose(meminfo);

    memory->ram.bytesTotal = memTotal * (uint64_t) 1024;
    if(memory->ram.bytesTotal == 0)
        ffStrbufAppendS(&memory->ram.error, "Failed to read MemTotal");
    else
        memory->ram.bytesUsed = (memTotal + shmem - memFree - buffers - cached - sReclaimable) * (uint64_t) 1024;

    memory->swap.bytesTotal = swapTotal * (uint64_t) 1024;
    memory->swap.bytesUsed = (swapTotal - swapFree) * (uint64_t) 1024;
}
