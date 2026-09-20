#include "memory.h"

#include <OS.h>

const char* ffDetectMemory(FFMemoryResult* ram) {
    system_info info;
    if (get_system_info(&info) != B_OK) {
        return "Error getting system info";
    }

    const uint32_t pageSizeShift = instance.state.platform.sysinfo.pageSizeShift;
    ram->bytesTotal = (uint64_t) info.max_pages << pageSizeShift;
    ram->bytesUsed = (uint64_t) info.used_pages << pageSizeShift;

    return nullptr;
}
