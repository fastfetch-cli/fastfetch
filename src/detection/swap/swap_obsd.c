#include "swap.h"
#include "util/FFlist.h"
#include "util/mallocHelper.h"

#include <sys/types.h>
#include <sys/swap.h>
#include <sys/param.h>
#include <unistd.h>

const char* ffDetectSwap(FFlist* result)
{
    int nswap = swapctl(SWAP_NSWAP, 0, 0);
    if (nswap < 0) return "swapctl(SWAP_NSWAP) failed";
    if (nswap == 0) return NULL;

    FF_AUTO_FREE struct swapent* swdev = malloc((uint32_t) nswap * sizeof(*swdev));

    if (swapctl(SWAP_STATS, swdev, nswap) < 0)
        return "swapctl(SWAP_STATS) failed";

    for (int i = 0; i < nswap; i++)
    {
        if (swdev[i].se_flags & SWF_ENABLE)
        {
            FFSwapResult* swap = ffListAdd(result);
            ffStrbufInitS(&swap->name, swdev[i].se_path);
            swap->bytesUsed = (uint64_t) swdev[i].se_inuse * DEV_BSIZE;
            swap->bytesTotal = (uint64_t) swdev[i].se_nblks * DEV_BSIZE;
        }
    }

    return NULL;
}
