#include "swap.h"
#include <sys/stat.h>
#include <sys/swap.h>
#include <limits.h>

enum { FFMaxNSwap = 8 };

const char* ffDetectSwap(FFlist* result)
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

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    for (int i = 0; i < size; ++i)
    {
        FFSwapResult* swap = ffListAdd(result);
        ffStrbufInitS(&swap->name, table->swt_ent[i].ste_path);
        swap->bytesTotal = (uint64_t) table->swt_ent[i].ste_pages * pageSize;
        swap->bytesUsed = swap->bytesTotal - (uint64_t) table->swt_ent[i].ste_free * pageSize;
    }

    return NULL;
}
