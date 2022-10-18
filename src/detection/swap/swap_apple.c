#include "swap.h"
#include "common/sysctl.h"

#include <mach/mach.h>

void ffDetectSwapImpl(FFMemoryStorage* swap)
{
    struct xsw_usage xsw;
    size_t size = sizeof(xsw);
    if(sysctlbyname("vm.swapusage", &xsw, &size, 0, 0) != 0)
    {
        ffStrbufAppendS(&swap->error, "Failed to read vm.swapusage");
        return;
    }

    swap->bytesTotal = xsw.xsu_total;
    swap->bytesUsed = xsw.xsu_used;
}
