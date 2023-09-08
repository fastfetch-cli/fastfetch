#include "uptime.h"

#include <time.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    struct timespec uptime;
    if (clock_gettime(CLOCK_BOOTTIME, &uptime) != 0)
        return "clock_gettime(CLOCK_BOOTTIME) failed";

    result->uptime = (uint64_t) uptime.tv_sec * 1000 + (uint64_t) uptime.tv_nsec / 1000000;

    struct timespec realTime;
    if(clock_gettime(CLOCK_REALTIME, &realTime) != 0)
        return "clock_gettime(CLOCK_REALTIME) failed";

    result->bootTime = (uint64_t) realTime.tv_sec * 1000 + (uint64_t) realTime.tv_nsec / 1000000 + result->uptime;

    return NULL;
}
