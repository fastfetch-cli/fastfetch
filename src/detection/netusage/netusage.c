#include "netusage.h"

#include "common/time.h"

const char* ffNetUsageGetIoCounters(FFlist* result);

static FFlist ioCounters1;
static uint64_t time1;

void ffPrepareNetUsage(void)
{
    ffListInit(&ioCounters1, sizeof(FFNetUsageIoCounters));
    ffNetUsageGetIoCounters(&ioCounters1);
    time1 = ffTimeGetNow();
}

const char* ffDetectNetUsage(FFlist* result)
{
    const char* error = NULL;
    if (time1 == 0)
    {
        ffListInit(&ioCounters1, sizeof(FFNetUsageIoCounters));
        error = ffNetUsageGetIoCounters(&ioCounters1);
        if (error)
            return error;
        time1 = ffTimeGetNow();
        ffTimeSleep(1000);
    }

    uint64_t time2 = ffTimeGetNow();
    while (time2 - time1 < 1000)
    {
        ffTimeSleep((uint32_t) (1000 - (time2 - time1)));
        time2 = ffTimeGetNow();
    }

    error = ffNetUsageGetIoCounters(result);
    if (error)
        return error;

    if (result->length != ioCounters1.length)
        return "Different number of network interfaces. Network change?";

    for (uint32_t i = 0; i < result->length; ++i)
    {
        FFNetUsageIoCounters* icPrev = (FFNetUsageIoCounters*)ffListGet(&ioCounters1, i);
        FFNetUsageIoCounters* icCurr = (FFNetUsageIoCounters*)ffListGet(result, i);
        if (!ffStrbufEqual(&icPrev->name, &icCurr->name))
            return "Network interface name changed";

        static_assert(sizeof(FFNetUsageIoCounters) - offsetof(FFNetUsageIoCounters, txBytes) == sizeof(uint64_t) * 8, "Unexpected struct FFNetUsageIoCounters layout");
        for (size_t off = offsetof(FFNetUsageIoCounters, txBytes); off < sizeof(FFNetUsageIoCounters); off += sizeof(uint64_t))
        {
            uint64_t* prevValue = (uint64_t*) ((uint8_t*) icPrev + off);
            uint64_t* currValue = (uint64_t*) ((uint8_t*) icCurr + off);
            uint64_t temp = *currValue;
            *currValue -= *prevValue;
            *currValue /= (time2 - time1) / 1000 /* seconds */;
            *prevValue = temp;
        }
    }
    time1 = time2;

    return NULL;
}
