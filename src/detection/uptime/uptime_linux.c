#include "uptime.h"

#include <sys/sysinfo.h>

const char* ffDetectUptime(uint64_t* result)
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        return "sysinfo() failed";
    *result = info.uptime;
    return NULL;
}
