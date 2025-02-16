#include "physicaldisk.h"

#include "common/io/io.h"

#include <OS.h>
#include <StorageDefs.h>
#include <Drivers.h>
#include <sys/ioctl.h>

static const char* detectDisk(int dfd, const char* diskType, const char* diskId, FFlist* result)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s/master/raw", diskId);
    FF_AUTO_CLOSE_FD int rawfd = openat(dfd, buffer, O_RDONLY);
    if (rawfd < 0)
    {
        snprintf(buffer, sizeof(buffer), "%s/raw", diskId);
        rawfd = openat(dfd, buffer, O_RDONLY);
        if (rawfd < 0) return "raw device file not found";
    }

    device_geometry geometry;
    if (ioctl(rawfd, B_GET_GEOMETRY, &geometry, sizeof(geometry)) < 0)
        return "ioctl(B_GET_GEOMETRY) failed";

    FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);

    char name[B_OS_NAME_LENGTH];
    if (ioctl(rawfd, B_GET_DEVICE_NAME, name, sizeof(name)) == 0)
        ffStrbufInitS(&device->name, name);
    else
    {
        // ioctl reports `not a tty` for NVME drives for some reason
        ffStrbufInitF(&device->name, "Unknown %s drive", diskType);
    }

    ffStrbufInitF(&device->devPath, "/dev/disk/%s/%s", diskType, buffer);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->revision);
    ffStrbufInitS(&device->interconnect, diskType);
    device->temperature = 0.0/0.0;
    device->type = FF_PHYSICALDISK_TYPE_NONE;
    if (geometry.read_only)
        device->type |= FF_PHYSICALDISK_TYPE_READONLY;
    if (geometry.removable)
        device->type |= FF_PHYSICALDISK_TYPE_REMOVABLE;
    device->size = (uint64_t) geometry.cylinder_count * geometry.sectors_per_track * geometry.bytes_per_sector;

    return NULL;
}

static const char* detectDiskType(int dfd, const char* diskType, FFlist* result)
{
    int newfd = openat(dfd, diskType, O_RDONLY);
    if (newfd < 0) return "openat(dfd, diskType) failed";

    FF_AUTO_CLOSE_DIR DIR* dir = fdopendir(newfd);
    if (!dir) return "fdopendir(newfd) failed";

    struct dirent* entry;
    while((entry = readdir(dir)))
    {
        if (entry->d_name[0] == '.') continue;
        detectDisk(newfd, diskType, entry->d_name, result);
    }
    return NULL;
}

const char* ffDetectPhysicalDisk(FFlist* result, FF_MAYBE_UNUSED FFPhysicalDiskOptions* options)
{
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/dev/disk");
    if (!dir) return "opendir(/dev/disk) failed";

    struct dirent* entry;
    while((entry = readdir(dir)))
    {
        if (entry->d_name[0] == '.') continue;
        detectDiskType(dirfd(dir), entry->d_name, result);
    }

    return NULL;
}
