#include "diskio.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <sys/disk.h>
#include <sys/sysctl.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    int mib[] = {CTL_HW, HW_DISKSTATS};
    size_t len;
    if (sysctl(mib, ARRAY_SIZE(mib), NULL, &len, NULL, 0) < 0)
        return "sysctl({HW_DISKSTATS}, NULL) failed";
    uint32_t nDrive = (uint32_t) (len / sizeof(struct diskstats));

    FF_AUTO_FREE struct diskstats* stats = malloc(len);

    if (sysctl(mib, ARRAY_SIZE(mib), stats, &len, NULL, 0) < 0)
        return "sysctl({HW_DISKSTATS}, stats) failed";

    for (uint32_t i = 0; i < nDrive; ++i)
    {
        struct diskstats* st = &stats[i];

        if (options->namePrefix.length && strncmp(st->ds_name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitF(&device->devPath, "/dev/%s", st->ds_name);
        ffStrbufInitS(&device->name, st->ds_name);
        device->bytesRead = st->ds_rbytes;
        device->readCount = st->ds_rxfer;
        device->bytesWritten = st->ds_wbytes;
        device->writeCount = st->ds_wxfer;
    }

    return NULL;
}
