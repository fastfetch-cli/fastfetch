#include "memory.h"
#include <unistd.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    ram->bytesTotal = sysconf(_SC_PHYS_PAGES) * instance.state.platform.sysinfo.pageSize;
    ram->bytesUsed = ram->bytesTotal - sysconf(_SC_AVPHYS_PAGES) * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
