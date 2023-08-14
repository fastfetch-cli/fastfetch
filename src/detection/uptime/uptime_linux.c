#include "uptime.h"

#include <sys/sysinfo.h>

const char* ffDetectUptime(uint64_t* result)
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        return "sysinfo() failed";
    *result = (uint64_t) info.uptime;
    return NULL;
}
