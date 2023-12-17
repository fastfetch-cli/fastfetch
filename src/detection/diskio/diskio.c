#include "diskio.h"

#include "common/time.h"

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options);

static FFlist ioCounters1;
static uint64_t time1;

void ffPrepareDiskIO(FFDiskIOOptions* options)
{
    ffListInit(&ioCounters1, sizeof(FFDiskIOResult));
    ffDiskIOGetIoCounters(&ioCounters1, options);
    time1 = ffTimeGetNow();
}

const char* ffDetectDiskIO(FFlist* result, FFDiskIOOptions* options)
{
    const char* error = NULL;
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
    while (time2 - time1 < 1000)
    {
        ffTimeSleep((uint32_t) (1000 - (time2 - time1)));
        time2 = ffTimeGetNow();
    }

    error = ffDiskIOGetIoCounters(result, options);
    if (error)
        return error;

    if (result->length != ioCounters1.length)
        return "Different number of physical disks. Hardware change?";

    for (uint32_t i = 0; i < result->length; ++i)
    {
        FFDiskIOResult* icPrev = (FFDiskIOResult*)ffListGet(&ioCounters1, i);
        FFDiskIOResult* icCurr = (FFDiskIOResult*)ffListGet(result, i);
        if (!ffStrbufEqual(&icPrev->name, &icCurr->name))
            return "Physical disk name changed";

        static_assert(sizeof(FFDiskIOResult) - offsetof(FFDiskIOResult, bytesRead) == sizeof(uint64_t) * 4, "Unexpected struct FFDiskIOResult layout");
        for (size_t off = offsetof(FFDiskIOResult, bytesRead); off < sizeof(FFDiskIOResult); off += sizeof(uint64_t))
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
