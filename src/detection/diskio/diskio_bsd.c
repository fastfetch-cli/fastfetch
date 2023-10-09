#include "diskio.h"

#include <devstat.h>
#include <memory.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    if (devstat_checkversion(NULL) < 0)
        return "devstat_checkversion() failed";

    struct statinfo stats = {
        .dinfo = (struct devinfo *)calloc(1, sizeof(struct devinfo)),
    };
    if (devstat_getdevs(NULL, &stats) < 0)
        return "devstat_getdevs() failed";

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

        ffStrbufInit(&device->type);
        switch (current->device_type & DEVSTAT_TYPE_IF_MASK)
        {
            case DEVSTAT_TYPE_IF_SCSI: ffStrbufAppendS(&device->type, "SCSI"); break;
            case DEVSTAT_TYPE_IF_IDE: ffStrbufAppendS(&device->type, "IDE"); break;
            case DEVSTAT_TYPE_IF_OTHER: ffStrbufAppendS(&device->type, "OTHER"); break;
        }
        device->bytesRead = current->bytes[DEVSTAT_READ];
        device->readCount = current->operations[DEVSTAT_READ];
        device->bytesWritten = current->bytes[DEVSTAT_WRITE];
        device->writeCount = current->operations[DEVSTAT_WRITE];
    }

    if (stats.dinfo->mem_ptr)
        free(stats.dinfo->mem_ptr);
    free(stats.dinfo);

    return NULL;
}
