#include "swap.h"

#include <sys/sysinfo.h>

void ffDetectSwap(FFMemoryStorage* swap)
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        ffStrbufAppendS(&swap->error, "sysinfo() failed");

    swap->bytesTotal = info.totalswap * (uint64_t) info.mem_unit;
    swap->bytesUsed = (info.totalswap - info.freeswap) * (uint64_t) info.mem_unit;
}
