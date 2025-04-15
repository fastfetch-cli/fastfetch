#include "uptime.h"
#include "common/time.h"

#include <realtimeapiset.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    // According to MSDN, this function only fails if it's called with NULL
    QueryUnbiasedInterruptTime(&result->uptime);
    result->uptime /= 10000; // Convert from 100-nanosecond intervals to milliseconds
    result->bootTime = ffTimeGetNow() - result->uptime;
    return NULL;
}
