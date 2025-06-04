#include "swap.h"

#include <OS.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    swap->bytesTotal = pageSize * (uint64_t) info.max_swap_pages;
    swap->bytesUsed = pageSize * (uint64_t) (info.max_swap_pages - info.free_swap_pages);

    return NULL;
}
