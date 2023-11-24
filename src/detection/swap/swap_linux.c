#include "swap.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    // #620
    FF_AUTO_CLOSE_FILE FILE* meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo) return "fopen(\"/proc/meminfo\", \"r\") failed";

    uint64_t swapTotal = 0, swapFree = 0;

    char* FF_AUTO_FREE line = NULL;
    size_t len = 0;
    uint8_t count = 0;

    while (getline(&line, &len, meminfo) != EOF)
    {
        if (line[0] == 'S')
        {
            if(sscanf(line, "SwapTotal: %" PRIu64, &swapTotal) > 0 || sscanf(line, "SwapFree: %" PRIu64, &swapFree) > 0)
                if (++count >= 2) goto done;
        }
    }

done:
    swap->bytesTotal = swapTotal * 1024lu;
    swap->bytesUsed = (swapTotal - swapFree) * 1024lu;

    return NULL;
}
