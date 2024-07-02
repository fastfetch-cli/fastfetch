#include "swap.h"
#include "common/sysctl.h"

#include <vm/vm_param.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    int mib[16];
    size_t mibsize = sizeof(mib) / sizeof(*mib);
    if (sysctlnametomib("vm.swap_info", mib, &mibsize) < 0)
        return "sysctlnametomib(\"vm.swap_info\") failed";

    swap->bytesUsed = swap->bytesTotal = 0;

    for (int n = 0; ; ++n)
    {
        mib[mibsize] = n;
        struct xswdev xsw;
        size_t size = sizeof(xsw);
        if (sysctl(mib, (uint32_t) (mibsize + 1), &xsw, &size, NULL, 0) < 0)
            break;
        if (xsw.xsw_version != XSWDEV_VERSION)
            return "xswdev version mismatch";
        swap->bytesUsed += (uint64_t) xsw.xsw_used;
        swap->bytesTotal += (uint64_t) xsw.xsw_nblks;
    }

    swap->bytesUsed *= instance.state.platform.sysinfo.pageSize;
    swap->bytesTotal *= instance.state.platform.sysinfo.pageSize;

    return NULL;
}
