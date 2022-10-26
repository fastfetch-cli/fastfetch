#include "swap.h"
#include "common/sysctl.h"

void ffDetectSwapImpl(FFMemoryStorage* swap)
{
    swap->bytesTotal = (uint64_t)ffSysctlGetInt64("vm.swap_total", 0);
    swap->bytesUsed = (uint64_t)ffSysctlGetInt64("vm.swap_reserved", 0);
}
