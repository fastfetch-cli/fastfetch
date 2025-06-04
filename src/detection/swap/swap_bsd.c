#include "swap.h"
#include "common/sysctl.h"

#include <vm/vm_param.h>
#include <sys/stat.h>

const char* ffDetectSwap(FFlist* result)
{
    int mib[16];
    size_t mibsize = ARRAY_SIZE(mib);
    if (sysctlnametomib("vm.swap_info", mib, &mibsize) < 0)
        return "sysctlnametomib(\"vm.swap_info\") failed";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    
    for (int n = 0; ; ++n)
    {
        mib[mibsize] = n;
        struct xswdev xsw;
        size_t size = sizeof(xsw);
        if (sysctl(mib, (uint32_t) (mibsize + 1), &xsw, &size, NULL, 0) < 0)
            break;
        if (xsw.xsw_version != XSWDEV_VERSION)
            return "xswdev version mismatch";
        
        FFSwapResult* swap = ffListAdd(result);
        ffStrbufInitF(&swap->name, "/dev/%s", devname(xsw.xsw_dev, S_IFCHR));
        swap->bytesUsed = (uint64_t) xsw.xsw_used * pageSize;
        swap->bytesTotal = (uint64_t) xsw.xsw_nblks * pageSize;
    }

    return NULL;
}
