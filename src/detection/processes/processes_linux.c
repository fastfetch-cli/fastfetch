#include "processes.h"

#include <sys/sysinfo.h>

const char* ffDetectProcesses(uint32_t* result)
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        return "sysinfo() failed";

    *result = (uint32_t) info.procs;
    return NULL;
}
