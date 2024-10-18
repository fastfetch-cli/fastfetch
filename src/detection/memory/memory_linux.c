#include "memory.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/meminfo", ARRAY_SIZE(buf) - 1, buf);
    if(nRead < 0)
        return "ffReadFileData(\"/proc/meminfo\", ARRAY_SIZE(buf)-1, buf)";
    buf[nRead] = '\0';

    uint64_t memTotal = 0,
             memAvailable = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0;

    char *token = NULL;
    if((token = strstr(buf, "MemTotal:")) != NULL)
        memTotal = strtoul(token + strlen("MemTotal:"), NULL, 10);
    else
        return "MemTotal not found in /proc/meminfo";

    if((token = strstr(buf, "MemAvailable:")) != NULL)
        memAvailable = strtoul(token + strlen("MemAvailable:"), NULL, 10);
    else
    {
        if((token = strstr(buf, "MemFree:")) != NULL)
            memFree = strtoul(token + strlen("MemFree:"), NULL, 10);

        if((token = strstr(buf, "Buffers:")) != NULL)
            buffers = strtoul(token + strlen("Buffers:"), NULL, 10);

        if((token = strstr(buf, "Cached:")) != NULL)
            cached = strtoul(token + strlen("Cached:"), NULL, 10);

        if((token = strstr(buf, "Shmem:")) != NULL)
            shmem = strtoul(token + strlen("Shmem:"), NULL, 10);

        if((token = strstr(buf, "SReclaimable:")) != NULL)
            sReclaimable = strtoul(token + strlen("SReclaimable:"), NULL, 10);

        memAvailable = memFree + buffers + cached + sReclaimable - shmem;
    }

    ram->bytesTotal = memTotal * 1024lu;
    ram->bytesUsed = (memTotal - memAvailable) * 1024lu;

    return NULL;
}
