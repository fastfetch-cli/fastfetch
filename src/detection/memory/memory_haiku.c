#include "memory.h"

#include <OS.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    ram->bytesTotal = pageSize * info.max_pages;
    ram->bytesUsed = pageSize * info.used_pages;

    return NULL;
}
