#include "swap.h"

#include <OS.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    swap->bytesTotal = B_PAGE_SIZE * (uint64_t) info.max_swap_pages;
    swap->bytesUsed = B_PAGE_SIZE * (uint64_t) (info.max_swap_pages - info.free_swap_pages);

    return NULL;
}
