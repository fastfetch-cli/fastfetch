#include "netio.h"

#include "common/time.h"

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options);

static FFlist ioCounters1;
static uint64_t time1;

void ffPrepareNetIO(FFNetIOOptions* options)
{
    if (options->detectTotal) return;

    ffListInit(&ioCounters1, sizeof(FFNetIOResult));
    ffNetIOGetIoCounters(&ioCounters1, options);
    time1 = ffTimeGetNow();
}

const char* ffDetectNetIO(FFlist* result, FFNetIOOptions* options)
{
    const char* error = NULL;

    if (options->detectTotal)
    {
        error = ffNetIOGetIoCounters(result, options);
        if (error)
            return error;
        return NULL;
    }

    if (time1 == 0)
    {
        ffListInit(&ioCounters1, sizeof(FFNetIOResult));
        error = ffNetIOGetIoCounters(&ioCounters1, options);
        if (error)
            return error;
        time1 = ffTimeGetNow();
    }

    if (ioCounters1.length == 0)
        return "No network interfaces found";

    uint64_t time2 = ffTimeGetNow();
    while (time2 - time1 < 1000)
    {
        ffTimeSleep((uint32_t) (1000 - (time2 - time1)));
        time2 = ffTimeGetNow();
    }

    error = ffNetIOGetIoCounters(result, options);
    if (error)
        return error;

    if (result->length != ioCounters1.length)
        return "Different number of network interfaces. Network change?";

    for (uint32_t i = 0; i < result->length; ++i)
    {
        FFNetIOResult* icPrev = (FFNetIOResult*)ffListGet(&ioCounters1, i);
        FFNetIOResult* icCurr = (FFNetIOResult*)ffListGet(result, i);
        if (!ffStrbufEqual(&icPrev->name, &icCurr->name))
            return "Network interface name changed";

        static_assert(sizeof(FFNetIOResult) - offsetof(FFNetIOResult, txBytes) == sizeof(uint64_t) * 8, "Unexpected struct FFNetIOResult layout");
        for (size_t off = offsetof(FFNetIOResult, txBytes); off < sizeof(FFNetIOResult); off += sizeof(uint64_t))
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
