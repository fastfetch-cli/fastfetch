#include "uptime.h"
#include "common/time.h"
#include "common/io/io.h"

#include <inttypes.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    // #620
    FF_AUTO_CLOSE_FILE FILE* uptime = fopen("/proc/uptime", "r");
    if (!uptime) return "fopen(\"/proc/uptime\", \"r\") failed";

    double sec;
    if (fscanf(uptime, "%lf", &sec) > 0)
        result->uptime = (uint64_t) (sec * 1000);
    else
        return "fscanf(\"%lf\", &sec) failed";

    result->bootTime = ffTimeGetNow() + result->uptime;

    return NULL;
}
