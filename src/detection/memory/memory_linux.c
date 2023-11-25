#include "memory.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    FF_AUTO_CLOSE_FILE FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
        return "Failed to open /proc/meminfo";

    char* FF_AUTO_FREE line = NULL;
    size_t len = 0;

    uint64_t memTotal = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0;
    uint8_t count = 0;

    while (getline(&line, &len, meminfo) != EOF)
    {
        switch (line[0])
        {
            case 'B':
                if (sscanf(line, "Buffers: %" PRIu64, &buffers) > 0)
                    if (++count >= 6) goto done;
                break;
            case 'C':
                if (sscanf(line, "Cached: %" PRIu64, &cached) > 0)
                    if (++count >= 6) goto done;
                break;
            case 'M':
                if(sscanf(line, "MemTotal: %" PRIu64, &memTotal) > 0 || sscanf(line, "MemFree: %" PRIu64, &memFree) > 0)
                    if (++count >= 6) goto done;
                break;
            case 'S':
                if(sscanf(line, "Shmem: %" PRIu64, &shmem) > 0 || sscanf(line, "SReclaimable: %" PRIu64, &sReclaimable) > 0)
                    if (++count >= 6) goto done;
                break;
        }
    }

done:
    ram->bytesTotal = memTotal * 1024lu;
    ram->bytesUsed = (memTotal + shmem - memFree - buffers - cached - sReclaimable) * 1024lu;

    return NULL;
}
