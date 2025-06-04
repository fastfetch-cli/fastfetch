#include "swap.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    // Ref: #620
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/swaps", ARRAY_SIZE(buf) - 1, buf);
    if(nRead <= 0)
        return "ffReadFileData(\"/proc/swaps\", ARRAY_SIZE(buf)-1, buf) failed";
    buf[nRead] = '\0';

    // Skip header
    char* line = memchr(buf, '\n', (size_t) nRead);

    while(line && *++line)
    {
        uint64_t total, used;
        if(sscanf(line, "%*[^\t]%" SCNu64 "%" SCNu64, &total, &used) != 2)
            return "Invalid /proc/swaps format found";

        swap->bytesTotal += total;
        swap->bytesUsed += used;

        line = memchr(line, '\n', (size_t) (nRead - (line - buf)));
    }

    swap->bytesTotal *= 1024u;
    swap->bytesUsed *= 1024u;

    return NULL;
}
