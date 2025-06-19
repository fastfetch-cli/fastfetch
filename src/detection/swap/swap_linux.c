#include "swap.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

static const char* detectByProcMeminfo(FFlist* result)
{
    // For Android
    // Ref: #620
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/meminfo", ARRAY_SIZE(buf) - 1, buf);
    if(nRead < 0)
        return "ffReadFileData(\"/proc/meminfo\", ARRAY_SIZE(buf)-1, buf)";
    buf[nRead] = '\0';

    uint64_t swapTotal = 0, swapFree = 0;

    char *token = NULL;
    if ((token = strstr(buf, "SwapTotal:")) != NULL)
        swapTotal = strtoul(token + strlen("SwapTotal:"), NULL, 10);

    if ((token = strstr(buf, "SwapFree:")) != NULL)
        swapFree = strtoul(token + strlen("SwapFree:"), NULL, 10);

    FFSwapResult* swap = ffListAdd(result);
    ffStrbufInitStatic(&swap->name, "Total");
    swap->bytesTotal = swapTotal * 1024lu;
    swap->bytesUsed = (swapTotal - swapFree) * 1024lu;

    return NULL;
}

static const char* detectByProcSwaps(FFlist* result)
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

const char* ffDetectSwap(FFlist* result)
{
    if (detectByProcSwaps(result) == NULL)
        return NULL;
    return detectByProcMeminfo(result);
}
