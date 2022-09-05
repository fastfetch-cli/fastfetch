#include "memory.h"

#include <stdlib.h>

void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    memory->bytesUsed = 0;
    memory->bytesTotal = 0;

    FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
        return;

    char* line = NULL;
    size_t len = 0;

    uint32_t memTotal = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0;

    while (getline(&line, &len, meminfo) != EOF)
    {
        sscanf(line, "MemTotal: %u", &memTotal);
        sscanf(line, "Shmem: %u", &shmem);
        sscanf(line, "MemFree: %u", &memFree);
        sscanf(line, "Buffers: %u", &buffers);
        sscanf(line, "Cached: %u", &cached);
        sscanf(line, "SReclaimable: %u", &sReclaimable);
    }

    if(line != NULL)
        free(line);

    fclose(meminfo);

    memory->bytesTotal = memTotal * (uint64_t) 1024;
    memory->bytesUsed = (memTotal + shmem - memFree - buffers - cached - sReclaimable) * (uint64_t) 1024;
}
