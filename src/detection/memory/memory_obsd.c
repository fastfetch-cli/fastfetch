#include "memory.h"
#include "common/sysctl.h"

#include <sys/vmmeter.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_PHYSMEM }, 2, &ram->bytesTotal, &length, NULL, 0) < 0)
        return "Failed to read hw.physmem";

    struct vmtotal vmtotal;
    length = sizeof(vmtotal);
    if (sysctl((int[]) {CTL_VM, VM_METER}, 2, &vmtotal, &length, NULL, 0) < 0)
        return "sysctl(VM_METER) failed";

    ram->bytesUsed = ram->bytesTotal - vmtotal.t_free * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
