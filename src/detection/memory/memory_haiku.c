#include "memory.h"

#include <OS.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    ram->bytesTotal = B_PAGE_SIZE * info.max_pages;
    ram->bytesUsed = B_PAGE_SIZE * info.used_pages;

    return NULL;
}
