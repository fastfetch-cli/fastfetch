#include "uptime.h"
#include "common/time.h"
#include "common/io/io.h"

#include <inttypes.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    // #620
    char buf[64];
    ssize_t nRead = ffReadFileData("/proc/uptime", sizeof(buf) - 1, buf);
    if(nRead < 0)
        return "ffReadFileData(\"/proc/uptime\", sizeof(buf) - 1, buf) failed";
    buf[nRead] = '\0';

    double sec;
    if(sscanf(buf, "%lf", &sec) > 0)
        result->uptime = (uint64_t) (sec * 1000);
    else
        return "sscanf(buf.chars, \"%lf\", &sec) failed";

    result->bootTime = ffTimeGetNow() + result->uptime;

    return NULL;
}
