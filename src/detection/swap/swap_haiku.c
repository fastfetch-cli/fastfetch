#include "swap.h"

#include <OS.h>

enum { FFMaxNSwap = 8 };

const char* ffDetectSwap(FFSwapResult* swap)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    swap->bytesTotal = B_PAGE_SIZE * info.max_swap_pages;
    swap->bytesUsed = B_PAGE_SIZE * (info.max_swap_pages - info.free_swap_pages);

    return NULL;
}
