#include "uptime.h"

#include <time.h>
#include <sys/sysctl.h>
#include <sys/time.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
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
    }

    {
        struct timespec realTime;
        if(clock_gettime(CLOCK_REALTIME, &realTime) != 0)
            return "clock_gettime(CLOCK_REALTIME) failed";
        uint64_t now = (uint64_t) realTime.tv_sec * 1000 + (uint64_t) realTime.tv_nsec / 1000000;
        result->uptime = now - result->bootTime;
    }

    return NULL;
}
