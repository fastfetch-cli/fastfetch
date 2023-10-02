#include "uptime.h"
#include "common/time.h"

#include <sysinfoapi.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    result->uptime = GetTickCount64();
    result->bootTime = ffTimeGetNow() - result->uptime;
    return NULL;
}
