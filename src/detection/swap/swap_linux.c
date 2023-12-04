#include "swap.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    // #620
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/meminfo", sizeof(buf) - 1, buf);
    if(nRead < 0)
        return "ffReadFileData(\"/proc/meminfo\", sizeof(buf)-1, buf)";
    buf[nRead] = '\0';

    uint64_t swapTotal = 0, swapFree = 0;

    char *token = NULL;
    if ((token = strstr(buf, "SwapTotal:")) != NULL)
        sscanf(token, "SwapTotal: %" PRIu64, &swapTotal);
    
    if ((token = strstr(buf, "SwapFree:")) != NULL)
        sscanf(token, "SwapFree: %" PRIu64, &swapFree);

    swap->bytesTotal = swapTotal * 1024lu;
    swap->bytesUsed = (swapTotal - swapFree) * 1024lu;

    return NULL;
}
