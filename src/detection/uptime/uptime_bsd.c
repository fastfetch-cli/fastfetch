#include "uptime.h"
#include "common/time.h"

#include <sys/sysctl.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    struct timeval bootTime;
    size_t bootTimeSize = sizeof(bootTime);
    if(sysctl(
        (int[]) {CTL_KERN, KERN_BOOTTIME}, 2,
        &bootTime, &bootTimeSize,
        NULL, 0
    ) != 0)
        return "sysctl({CTL_KERN, KERN_BOOTTIME}) failed";

    result->bootTime = (uint64_t) bootTime.tv_sec * 1000 + (uint64_t) bootTime.tv_usec / 1000;
    result->uptime = ffTimeGetNow() - result->bootTime;

    return NULL;
}
