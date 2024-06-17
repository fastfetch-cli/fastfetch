#include "uptime.h"
#include "common/time.h"
#include <utmpx.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    struct utmpx* ut;

    setutxent();
    while (NULL != (ut = getutxent()))
    {
        if (ut->ut_type == BOOT_TIME)
        {
            result->bootTime = (uint64_t) ut->ut_tv.tv_sec * 1000 + (uint64_t) ut->ut_tv.tv_usec / 1000000;
            result->uptime = ffTimeGetNow() - result->bootTime;
            break;
        }
    }
    endutxent();
    return NULL;
}
