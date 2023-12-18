#include "diskio.h"
#include "util/stringUtils.h"

#include <devstat.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <libgeom.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    struct gmesh geomTree;
    if (geom_gettree(&geomTree) < 0)
        return "geom_gettree() failed";

    if (geom_stats_open() < 0)
        return "geom_stats_open() failed";

    void* snap = geom_stats_snapshot_get();
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
