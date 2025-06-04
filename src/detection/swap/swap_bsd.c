#include "swap.h"
#include "common/sysctl.h"

#include <vm/vm_param.h>
#include <sys/stat.h>
#include <sys/param.h>

static void addSwapEntry(FFlist* result, struct xswdev* xsw, uint32_t pageSize)
{
    if (xsw->xsw_nblks == 0) // DFBSD reports some /dev/wdog devices with nblks == 0
        return;

    FFSwapResult* swap = ffListAdd(result);
    if (xsw->xsw_dev == NODEV)
        ffStrbufInitStatic(&swap->name, "[NFS]");
    else
        ffStrbufInitF(&swap->name, "/dev/%s", devname(xsw->xsw_dev, S_IFCHR));
    swap->bytesUsed = (uint64_t) xsw->xsw_used * pageSize;
    swap->bytesTotal = (uint64_t) xsw->xsw_nblks * pageSize;
}

#if __DragonFly__

const char* ffDetectSwap(FFlist* result)
{
    struct xswdev xsws[32];
    size_t size = sizeof(xsws);
    if (sysctlbyname("vm.swap_info_array", xsws, &size, NULL, 0) < 0)
        return "sysctlbyname(\"vm.swap_info_array\") failed";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;

    size_t count = size / sizeof(struct xswdev);
    if (count == 0)
        return NULL;

    if (xsws->xsw_version != XSWDEV_VERSION)
        return "xswdev version mismatch";

    for (uint32_t i = 0; i < count; ++i)
        addSwapEntry(result, &xsws[i], pageSize);

    return NULL;
}

#elif __FreeBSD__

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

        addSwapEntry(result, &xsw, pageSize);
    }

    return NULL;
}

#endif
