#include "swap.h"

#include <sys/sysinfo.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        return "sysinfo() failed";

    swap->bytesTotal = info.totalswap * (uint64_t) info.mem_unit;
    swap->bytesUsed = (info.totalswap - info.freeswap) * (uint64_t) info.mem_unit;

    return NULL;
}
