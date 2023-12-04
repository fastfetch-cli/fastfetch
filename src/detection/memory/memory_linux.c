#include "memory.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/meminfo", sizeof(buf) - 1, buf);
    if(nRead < 0)
        return "ffReadFileData(\"/proc/meminfo\", sizeof(buf)-1, buf)";
    buf[nRead] = '\0';

    uint64_t memTotal = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0;
    
    char *token = NULL;
    if((token = strstr(buf, "MemTotal:")) != NULL)
        sscanf(token, "MemTotal: %" PRIu64, &memTotal);

    if((token = strstr(buf, "MemFree:")) != NULL)
        sscanf(token, "MemFree: %" PRIu64, &memFree);

    if((token = strstr(buf, "Buffers:")) != NULL)
        sscanf(token, "Buffers: %" PRIu64, &buffers);
    
    if((token = strstr(buf, "Cached:")) != NULL)
        sscanf(token, "Cached: %" PRIu64, &cached);
    
    if((token = strstr(buf, "Shmem:")) != NULL)
        sscanf(token, "Shmem: %" PRIu64, &shmem);
    
    if((token = strstr(buf, "SReclaimable:")) != NULL)
        sscanf(token, "SReclaimable: %" PRIu64, &sReclaimable);

    ram->bytesTotal = memTotal * 1024lu;
    ram->bytesUsed = (memTotal + shmem - memFree - buffers - cached - sReclaimable) * 1024lu;

    return NULL;
}
