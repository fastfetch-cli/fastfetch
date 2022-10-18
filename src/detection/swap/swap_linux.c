#include "swap.h"

#include <stdlib.h>
#include <string.h>

void ffDetectSwapImpl(FFMemoryStorage* swap)
{
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
    {
        ffStrbufAppendS(&swap->error, "Failed to open /proc/meminfo");
        return;
    }

    char* line = NULL;
    size_t len = 0;

    uint32_t swapTotal = 0,
             swapFree = 0;

    while (getline(&line, &len, meminfo) != EOF)
    {
        if(!sscanf(line, "SwapTotal: %u", &swapTotal))
            sscanf(line, "SwapFree: %u", &swapFree);
    }

    if(line != NULL)
        free(line);

    fclose(meminfo);

    swap->bytesTotal = swapTotal * (uint64_t) 1024;
    swap->bytesUsed = (swapTotal - swapFree) * (uint64_t) 1024;
}
