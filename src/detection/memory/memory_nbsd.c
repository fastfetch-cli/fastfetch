#include "memory.h"
#include "common/sysctl.h"

#include <sys/param.h>
#include <uvm/uvm_extern.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    struct uvmexp_sysctl buf;
    size_t length = sizeof(buf);
    if (sysctl((int[]){ CTL_VM, VM_UVMEXP2 }, 2, &buf, &length, NULL, 0) < 0)
        return "sysctl(CTL_VM, VM_UVMEXP2) failed";

    ram->bytesTotal = (uint64_t) buf.npages * instance.state.platform.sysinfo.pageSize;
    ram->bytesUsed = ((uint64_t) buf.active + (uint64_t) buf.inactive + (uint64_t) buf.wired) * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
