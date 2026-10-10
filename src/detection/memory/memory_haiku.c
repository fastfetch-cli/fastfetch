#include "memory.h"
#include "common/debug.h"

#include <OS.h>

const char* ffDetectMemory(FFMemoryResult* ram) {
    system_info info;
    status_t status = get_system_info(&info);
    if (status != B_OK) {
        FF_DEBUG("get_system_info() failed: %d", (int) status);
        return "Error getting system info";
    }

    const uint32_t pageSizeShift = instance.state.platform.sysinfo.pageSizeShift;
    ram->bytesTotal = (uint64_t) info.max_pages << pageSizeShift;
    ram->bytesUsed = (uint64_t) info.used_pages << pageSizeShift;

    return nullptr;
}
