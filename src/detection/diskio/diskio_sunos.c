#include "diskio.h"
#include "util/stringUtils.h"
#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    __attribute__((__cleanup__(kstatFreeWrap))) kstat_ctl_t* kc = kstat_open();
    if (!kc)
        return "kstat_open() failed";

    for (kstat_t* ks = kc->kc_chain; ks; ks = ks->ks_next)
    {
        if (ks->ks_type != KSTAT_TYPE_IO || !ffStrEquals(ks->ks_class, "disk")) continue;

        if (options->namePrefix.length && strncmp(ks->ks_name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        kstat_io_t kio;
        if (kstat_read(kc, ks, &kio) < 0)
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitS(&device->devPath, ks->ks_name);
        device->bytesRead = kio.nread;
        device->readCount = kio.reads;
        device->bytesWritten = kio.nwritten;
        device->writeCount = kio.writes;
        ffStrbufInitS(&device->name, ks->ks_name);
    }

    return NULL;
}
