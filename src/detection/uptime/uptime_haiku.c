#include "uptime.h"
#include "common/time.h"

const char* ffDetectUptime(FFUptimeResult* result)
{
    result->uptime = (uint64_t) system_time() / 1000;
    result->bootTime = ffTimeGetNow() - result->uptime;
    return NULL;
}
