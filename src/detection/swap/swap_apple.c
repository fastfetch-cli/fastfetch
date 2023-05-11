#include "swap.h"

#include <mach/mach.h>
#include <sys/sysctl.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    struct xsw_usage xsw;
    size_t size = sizeof(xsw);
    if(sysctl((int[]){ CTL_VM, VM_SWAPUSAGE }, 2, &xsw, &size, NULL, 0) != 0)
        return "Failed to read vm.swapusage";

    swap->bytesTotal = xsw.xsu_total;
    swap->bytesUsed = xsw.xsu_used;
    return NULL;
}
