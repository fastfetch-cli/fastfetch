#include "diskio.h"

#if __has_include(<libgeom.h>)

#include "util/stringUtils.h"

#include <devstat.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <libgeom.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    __attribute__((__cleanup__(geom_deletetree)))
    struct gmesh geomTree = {};
    if (geom_gettree(&geomTree) < 0)
        return "geom_gettree() failed";

    if (geom_stats_open() < 0)
        return "geom_stats_open() failed";

    void* snap = geom_stats_snapshot_get();
    if (!snap)
        return "geom_stats_snapshot_get() failed";

    struct devstat* snapIter;
    while ((snapIter = geom_stats_snapshot_next(snap)) != NULL)
    {
        if (snapIter->device_type & DEVSTAT_TYPE_PASS)
            continue;
        struct gident* geomId = geom_lookupid(&geomTree, snapIter->id);
        if (geomId == NULL)
            continue;
        if (geomId->lg_what != ISPROVIDER)
            continue;
        struct gprovider* provider = (struct gprovider*) geomId->lg_ptr;
        if (provider->lg_geom->lg_rank != 1)
            continue;

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
        for (struct gconfig* ptr = provider->lg_config.lh_first; ptr; ptr = ptr->lg_config.le_next)
        {
            if (ffStrEquals(ptr->lg_name, "descr"))
                ffStrbufSetS(&name, ptr->lg_val);
        }
        if (name.length == 0)
            ffStrbufSetS(&name, provider->lg_name);

        if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix))
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitF(&device->devPath, "/dev/%s", provider->lg_name);
        device->bytesRead = snapIter->bytes[DEVSTAT_READ];
        device->readCount = snapIter->operations[DEVSTAT_READ];
        device->bytesWritten = snapIter->bytes[DEVSTAT_WRITE];
        device->writeCount = snapIter->operations[DEVSTAT_WRITE];
        ffStrbufInitMove(&device->name, &name);
    }

    geom_stats_snapshot_free(snap);
    geom_stats_close();

    return NULL;
}

#else

#include <devstat.h>
#include <memory.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    if (checkversion() < 0)
        return "checkversion() failed";

    struct statinfo stats = {
        .dinfo = (struct devinfo *)calloc(1, sizeof(struct devinfo)),
    };
    if (getdevs(&stats) < 0)
        return "getdevs() failed";

    for (int i = 0; i < stats.dinfo->numdevs; i++)
    {
        struct devstat* current = &stats.dinfo->devices[i];
        if (current->device_type & DEVSTAT_TYPE_PASS)
            continue;

        char deviceName[128];
        snprintf(deviceName, sizeof(deviceName), "%s%d", current->device_name, current->unit_number);

        if (options->namePrefix.length && strncmp(deviceName, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitS(&device->name, deviceName);
        ffStrbufInitF(&device->devPath, "/dev/%s", deviceName);
        device->bytesRead = current->bytes_read;
        device->readCount = current->num_reads;
        device->bytesWritten = current->bytes_written;
        device->writeCount = current->num_writes;
    }

    free(stats.dinfo->mem_ptr);
    free(stats.dinfo);

    return NULL;
}

#endif
