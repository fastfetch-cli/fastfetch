#include "swap.h"
#include <sys/stat.h>
#include <sys/swap.h>
#include <limits.h>

enum { FFMaxNSwap = 8 };

const char* ffDetectSwap(FFSwapResult* swap)
{
    char strings[FFMaxNSwap][PATH_MAX];
    uint8_t buffer[sizeof(swaptbl_t) + sizeof(swapent_t) * (FFMaxNSwap - 1)] = {};
    swaptbl_t* table = (swaptbl_t*) buffer;
    table->swt_n = FFMaxNSwap;
    for (int i = 0; i < FFMaxNSwap; ++i)
        table->swt_ent[i].ste_path = strings[i];

    int size = swapctl(SC_LIST, table);
    if (size < 0)
        return "swapctl() failed";

    swap->bytesTotal = swap->bytesUsed = 0;

    for (int i = 0; i < size; ++i)
    {
        swap->bytesTotal += (uint64_t) table->swt_ent[i].ste_pages;
        swap->bytesUsed += (uint64_t) table->swt_ent[i].ste_free;
    }
    swap->bytesUsed = swap->bytesTotal - swap->bytesUsed;
    swap->bytesTotal *= instance.state.platform.sysinfo.pageSize;
    swap->bytesUsed *= instance.state.platform.sysinfo.pageSize;

    return NULL;
}
