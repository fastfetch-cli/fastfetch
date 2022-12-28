#include "uptime.h"

#include <sys/sysinfo.h>

uint64_t ffDetectUptime()
{
    struct sysinfo info;
    if(sysinfo(&info) != 0)
        return 0;
    return (uint32_t) info.uptime;
}
