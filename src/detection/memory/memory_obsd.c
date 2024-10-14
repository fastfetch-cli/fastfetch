#include "memory.h"
#include "common/sysctl.h"

#include <sys/param.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    struct uvmexp buf;
    size_t length = sizeof(buf);
    if (sysctl((int[]){ CTL_VM, VM_UVMEXP }, 2, &buf, &length, NULL, 0) < 0)
        return "sysctl(CTL_VM, VM_UVMEXP) failed";

    ram->bytesTotal = (uint64_t) buf.npages * instance.state.platform.sysinfo.pageSize;
    ram->bytesUsed = ram->bytesTotal - (uint64_t) buf.free * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
