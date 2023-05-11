#include "swap.h"
#include "common/sysctl.h"

const char* ffDetectSwap(FFSwapResult* swap)
{
    swap->bytesTotal = (uint64_t)ffSysctlGetInt64("vm.swap_total", 0);
    swap->bytesUsed = (uint64_t)ffSysctlGetInt64("vm.swap_reserved", 0);
    return NULL;
}
