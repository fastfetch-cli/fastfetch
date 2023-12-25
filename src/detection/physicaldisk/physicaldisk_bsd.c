#include "physicaldisk.h"
#include "util/stringUtils.h"

#include <devstat.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <libgeom.h>

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options)
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
        FFPhysicalDiskType type = FF_PHYSICALDISK_TYPE_NONE;
        for (struct gconfig* ptr = provider->lg_config.lh_first; ptr; ptr = ptr->lg_config.le_next)
        {
            if (ffStrEquals(ptr->lg_name, "descr"))
                ffStrbufSetS(&name, ptr->lg_val);
            else if (ffStrEquals(ptr->lg_name, "rotationrate") && !ffStrEquals(ptr->lg_val, "unknown"))
                type |= ffStrEquals(ptr->lg_val, "0") ? FF_PHYSICALDISK_TYPE_SSD : FF_PHYSICALDISK_TYPE_HDD;
            else if (ffStrEquals(ptr->lg_name, "ident"))
                ffStrbufSetS(&identifier, ptr->lg_val);
        }

        if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix))
            continue;

        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        ffStrbufInitF(&device->devPath, "/dev/%s", provider->lg_name);
        ffStrbufInitMove(&device->serial, &identifier);
        ffStrbufInit(&device->revision);
        ffStrbufInit(&device->interconnect);
        switch (snapIter->device_type & DEVSTAT_TYPE_IF_MASK)
        {
            case DEVSTAT_TYPE_IF_SCSI: ffStrbufAppendS(&device->interconnect, "SCSI"); break;
            case DEVSTAT_TYPE_IF_IDE: ffStrbufAppendS(&device->interconnect, "IDE"); break;
            case DEVSTAT_TYPE_IF_OTHER: ffStrbufAppendS(&device->interconnect, "OTHER"); break;
        }
        device->size = (uint64_t) provider->lg_mediasize;
        ffStrbufInitMove(&device->name, &name);

        int acr = 1, acw = 1; // TODO: find some documents to verify the usage
        if (sscanf(provider->lg_mode, "r%dw%de%*d", &acr, &acw) == 2 && acr)
            type |= acw ? FF_PHYSICALDISK_TYPE_READONLY : FF_PHYSICALDISK_TYPE_READWRITE;

        device->type = type;
    }

    geom_stats_snapshot_free(snap);
    geom_stats_close();

    return NULL;
}
