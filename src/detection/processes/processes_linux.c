#include "processes.h"

#include <sys/sysinfo.h>

uint32_t ffDetectProcesses(FFstrbuf* error)
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        ffStrbufAppendS(error, "sysinfo() failed");
    return (uint32_t) info.procs;
}
