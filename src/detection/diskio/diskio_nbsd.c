#include "diskio.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <sys/iostat.h>
#include <sys/sysctl.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    int mib[] = {CTL_HW, HW_IOSTATS, sizeof(struct io_sysctl)};
    size_t len;
    if (sysctl(mib, ARRAY_SIZE(mib), NULL, &len, NULL, 0) < 0)
        return "sysctl({HW_IOSTATS}, NULL) failed";
    uint32_t nDrive = (uint32_t) (len / sizeof(struct io_sysctl));

    FF_AUTO_FREE struct io_sysctl* stats = malloc(len);

    if (sysctl(mib, ARRAY_SIZE(mib), stats, &len, NULL, 0) < 0)
        return "sysctl({HW_IOSTATS}, stats) failed";

    for (uint32_t i = 0; i < nDrive; ++i)
    {
        struct io_sysctl* st = &stats[i];

        if (options->namePrefix.length && strncmp(st->name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitF(&device->devPath, "/dev/%s", st->name);
        ffStrbufInitS(&device->name, st->name);
        device->bytesRead = st->rbytes;
        device->readCount = st->rxfer;
        device->bytesWritten = st->wbytes;
        device->writeCount = st->wxfer;
    }

    return NULL;
}
