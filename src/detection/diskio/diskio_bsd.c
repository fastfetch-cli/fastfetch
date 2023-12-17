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

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateS(provider->lg_name);
        FF_STRBUF_AUTO_DESTROY identifier = ffStrbufCreate();
        FFDiskIOPhysicalType type = FF_DISKIO_PHYSICAL_TYPE_UNKNOWN;
        for (struct gconfig* ptr = provider->lg_config.lh_first; ptr; ptr = ptr->lg_config.le_next)
        {
            if (ffStrEquals(ptr->lg_name, "descr"))
                ffStrbufSetS(&name, ptr->lg_val);
            else if (ffStrEquals(ptr->lg_name, "rotationrate") && !ffStrEquals(ptr->lg_val, "unknown"))
                type = ffStrEquals(ptr->lg_val, "0") ? FF_DISKIO_PHYSICAL_TYPE_SSD : FF_DISKIO_PHYSICAL_TYPE_HDD;
            else if (ffStrEquals(ptr->lg_name, "ident"))
                ffStrbufSetS(&identifier, ptr->lg_val);
        }

        if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix))
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitF(&device->devPath, "/dev/%s", provider->lg_name);
        ffStrbufInitMove(&device->serial, &identifier);
        ffStrbufInit(&device->interconnect);
        device->removable = false;
        switch (snapIter->device_type & DEVSTAT_TYPE_IF_MASK)
        {
            case DEVSTAT_TYPE_IF_SCSI: ffStrbufAppendS(&device->interconnect, "SCSI"); break;
            case DEVSTAT_TYPE_IF_IDE: ffStrbufAppendS(&device->interconnect, "IDE"); break;
            case DEVSTAT_TYPE_IF_OTHER: ffStrbufAppendS(&device->interconnect, "OTHER"); break;
        }
        device->bytesRead = snapIter->bytes[DEVSTAT_READ];
        device->readCount = snapIter->operations[DEVSTAT_READ];
        device->bytesWritten = snapIter->bytes[DEVSTAT_WRITE];
        device->writeCount = snapIter->operations[DEVSTAT_WRITE];
        device->size = (uint64_t) provider->lg_mediasize;
        ffStrbufInitMove(&device->name, &name);
        device->type = type;
    }

    geom_stats_snapshot_free(snap);
    geom_stats_close();

    return NULL;
}
