#include "uptime.h"
#include "common/time.h"
#include "common/io/io.h"

#include <inttypes.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    #ifndef __ANDROID__ // cat: /proc/uptime: Permission denied

    // #620
    char buf[64];
    ssize_t nRead = ffReadFileData("/proc/uptime", ARRAY_SIZE(buf) - 1, buf);
    if(nRead > 0)
    {
        buf[nRead] = '\0';

        char *err = NULL;
        double sec = strtod(buf, &err);
        if(err != buf)
        {
            result->uptime = (uint64_t) (sec * 1000);
            result->bootTime = ffTimeGetNow() - result->uptime;
            return NULL;
        }
    }

    #endif

    struct timespec uptime;
    if (clock_gettime(CLOCK_BOOTTIME, &uptime) != 0)
        return "clock_gettime(CLOCK_BOOTTIME) failed";

    result->uptime = (uint64_t) uptime.tv_sec * 1000 + (uint64_t) uptime.tv_nsec / 1000000;
    result->bootTime = ffTimeGetNow() - result->uptime;

    return NULL;
}
