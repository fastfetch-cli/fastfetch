#include "swap.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectSwap(FFlist* result)
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
        char name[256];
        if(sscanf(line, "%255s %*[^\t]%" SCNu64 "%" SCNu64, name, &total, &used) != 3)
            return "Invalid /proc/swaps format found";

        uint32_t nameLen = (uint32_t) strnlen(name, sizeof(name));
        FFSwapResult* swap = ffListAdd(result);
        ffStrbufInitA(&swap->name, nameLen);
        for (size_t i = 0; i < nameLen; ++i)
        {
            if(name[i] == '\\')
            {
                char octal[4] = { name[i + 1], name[i + 2], name[i + 3], '\0' };
                ffStrbufAppendC(&swap->name, (char) strtol(octal, NULL, 8));
                i += 3;
            }
            else
                ffStrbufAppendC(&swap->name, name[i]);
        }
        swap->bytesTotal = total * 1024u;
        swap->bytesUsed = used * 1024u;

        line = memchr(line, '\n', (size_t) (nRead - (line - buf)));
    }

    return NULL;
}
