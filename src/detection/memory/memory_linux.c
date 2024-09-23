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

    uint64_t memTotal = 0, memAvailable = 0, shmem = 0, memFree = 0, buffers = 0, cached = 0, sReclaimable = 0;
    const char* keys[] = {"MemTotal:", "MemAvailable:", "MemFree:", "Buffers:", "Cached:", "Shmem:", "SReclaimable:"};
    uint64_t* values[] = {&memTotal, &memAvailable, &memFree, &buffers, &cached, &shmem, &sReclaimable};

    for(size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); ++i)
    {
        char* token = strstr(buf, keys[i]);
        if(token)
            *values[i] = strtoul(token + strlen(keys[i]), NULL, 10);
    }

    if(!memTotal)
        return "MemTotal not found in /proc/meminfo";

    if(!memAvailable)
        memAvailable = memFree + buffers + cached + sReclaimable - shmem;

    ram->bytesTotal = memTotal * 1024lu;
    ram->bytesUsed = (memTotal - memAvailable) * 1024lu;

    return NULL;
}
