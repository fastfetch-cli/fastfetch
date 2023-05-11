#include "memory.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <string.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
        return "Failed to open /proc/meminfo";

    char* FF_AUTO_FREE line = NULL;
    size_t len = 0;

    uint32_t memTotal = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0;

    while (getline(&line, &len, meminfo) != EOF)
    {
        if(!sscanf(line, "MemTotal: %u", &memTotal))
        if(!sscanf(line, "Shmem: %u", &shmem))
        if(!sscanf(line, "MemFree: %u", &memFree))
        if(!sscanf(line, "Buffers: %u", &buffers))
        if(!sscanf(line, "Cached: %u", &cached))
            sscanf(line, "SReclaimable: %u", &sReclaimable);
    }

    fclose(meminfo);

    ram->bytesTotal = memTotal * (uint64_t) 1024;
    if(ram->bytesTotal == 0)
        return "Failed to read MemTotal";
    else
        ram->bytesUsed = (memTotal + shmem - memFree - buffers - cached - sReclaimable) * (uint64_t) 1024;

    return NULL;
}
