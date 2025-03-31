#include "swap.h"
#include "util/mallocHelper.h"

#include <sys/types.h>
#include <sys/swap.h>
#include <sys/param.h>
#include <unistd.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    int nswap = swapctl(SWAP_NSWAP, 0, 0);
    if (nswap < 0) return "swapctl(SWAP_NSWAP) failed";
    if (nswap == 0) return NULL;

    FF_AUTO_FREE struct swapent* swdev = malloc((uint32_t) nswap * sizeof(*swdev));

    if (swapctl(SWAP_STATS, swdev, nswap) < 0)
        return "swapctl(SWAP_STATS) failed";

    uint64_t swapTotal = 0, swapUsed = 0;
    for (int i = 0; i < nswap; i++)
    {
        if (swdev[i].se_flags & SWF_ENABLE)
        {
            swapUsed += (uint64_t) swdev[i].se_inuse;
            swapTotal += (uint64_t) swdev[i].se_nblks;
        }
    }
    swap->bytesUsed = swapUsed * DEV_BSIZE;
    swap->bytesTotal = swapTotal * DEV_BSIZE;

    return NULL;
}
