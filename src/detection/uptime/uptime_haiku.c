#include "uptime.h"
#include "common/time.h"

#include <OS.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    result->uptime = system_time() / 1000;
    result->bootTime = (real_time_clock_usecs() / 1000) - result->uptime;
    return NULL;
}
