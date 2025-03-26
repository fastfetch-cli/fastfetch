#include "diskio.h"

#include "common/time.h"

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options);

static FFlist ioCounters1;
static uint64_t time1;

void ffPrepareDiskIO(FFDiskIOOptions* options)
{
    if (options->detectTotal) return;

    ffListInit(&ioCounters1, sizeof(FFDiskIOResult));
    ffDiskIOGetIoCounters(&ioCounters1, options);
    time1 = ffTimeGetNow();
}

const char* ffDetectDiskIO(FFlist* result, FFDiskIOOptions* options)
{
    const char* error = NULL;

    if (options->detectTotal)
    {
        error = ffDiskIOGetIoCounters(result, options);
        if (error)
            return error;
        return NULL;
    }

    if (time1 == 0)
    {
        ffListInit(&ioCounters1, sizeof(FFDiskIOResult));
        error = ffDiskIOGetIoCounters(&ioCounters1, options);
        if (error)
            return error;
        time1 = ffTimeGetNow();
    }

    if (ioCounters1.length == 0)
        return "No physical disk found";

    uint64_t time2 = ffTimeGetNow();
    while (time2 - time1 < options->waitTime)
    {
        ffTimeSleep((uint32_t) (options->waitTime - (time2 - time1)));
        time2 = ffTimeGetNow();
    }

    error = ffDiskIOGetIoCounters(result, options);
    if (error)
        return error;

    if (result->length != ioCounters1.length)
        return "Different number of physical disks. Hardware change?";

    for (uint32_t i = 0; i < result->length; ++i)
    {
        FFDiskIOResult* icPrev = FF_LIST_GET(FFDiskIOResult, ioCounters1, i);
        FFDiskIOResult* icCurr = FF_LIST_GET(FFDiskIOResult, *result, i);
        if (!ffStrbufEqual(&icPrev->devPath, &icCurr->devPath))
            return "Physical disk device path changed";

        static_assert(sizeof(FFDiskIOResult) - offsetof(FFDiskIOResult, bytesRead) == sizeof(uint64_t) * 4, "Unexpected struct FFDiskIOResult layout");
        for (size_t off = offsetof(FFDiskIOResult, bytesRead); off < sizeof(FFDiskIOResult); off += sizeof(uint64_t))
        {
            uint64_t* prevValue = (uint64_t*) ((uint8_t*) icPrev + off);
            uint64_t* currValue = (uint64_t*) ((uint8_t*) icCurr + off);
            uint64_t temp = *currValue;
            *currValue -= *prevValue;
            *currValue /= (time2 - time1) / 1000 /* seconds */;

            // For next function call
            *prevValue = temp;
        }
    }

    // For next function call
    time1 = time2;
    // Leak ioCounters1 here

    return NULL;
}
