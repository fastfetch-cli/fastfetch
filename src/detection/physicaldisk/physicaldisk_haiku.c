#include "physicaldisk.h"

#include "common/io.h"
#include "common/stringUtils.h"

#include <OS.h>
#include <StorageDefs.h>
#include <Drivers.h>
#include <sys/ioctl.h>

static const char* searchRawDeviceFile(FFstrbuf* path, const char* diskType, FFlist* result, FFPhysicalDiskOptions* options) {
    FF_AUTO_CLOSE_DIR DIR* dir = opendir(path->chars);
    if (!dir) {
        return "detectDiskType: opendir() failed";
    }
    uint32_t baseLen = path->length;

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        ffStrbufAppendC(path, '/');
        ffStrbufAppendS(path, entry->d_name);

        struct stat st;
        if (stat(path->chars, &st) != 0) {
            ffStrbufSubstrBefore(path, baseLen);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            searchRawDeviceFile(path, diskType, result, options);
        } else if (ffStrEquals(entry->d_name, "raw")) {
            FF_AUTO_CLOSE_FD int rawfd = open(path->chars, O_RDONLY | O_CLOEXEC);
            if (rawfd < 0) {
                continue;
            }

            device_geometry geometry;
            if (ioctl(rawfd, B_GET_GEOMETRY, &geometry, sizeof(geometry)) < 0) {
                continue;
            }

            char name[B_OS_NAME_LENGTH];
            if (ioctl(rawfd, B_GET_DEVICE_NAME, name, sizeof(name)) != 0) {
                // ioctl reports `not a tty` for NVME drives for some reason
                snprintf(name, sizeof(name), "Unknown %s drive", diskType);
            }

            if (options->namePrefix.length && strncmp(name, options->namePrefix.chars, options->namePrefix.length) != 0) {
                continue;
            }

            FFPhysicalDiskType type = FF_PHYSICALDISK_TYPE_NONE;
            uint64_t size = (uint64_t) geometry.cylinder_count * geometry.head_count * geometry.sectors_per_track * geometry.bytes_per_sector;
            if (size == 0) {
                if (options->hideType & FF_PHYSICALDISK_TYPE_UNKNOWN) {
                    continue;
                }

                type |= FF_PHYSICALDISK_TYPE_UNKNOWN;
            }

            FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
            ffStrbufInitS(&device->name, name);
            ffStrbufInitCopy(&device->devPath, path);
            ffStrbufInit(&device->serial);
            ffStrbufInit(&device->revision);
            ffStrbufInitS(&device->interconnect, diskType);
            device->temperature = FF_PHYSICALDISK_TEMP_UNSET;
            device->type = type |
                (geometry.read_only ? FF_PHYSICALDISK_TYPE_READONLY : FF_PHYSICALDISK_TYPE_READWRITE) |
                (geometry.removable ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED);
            device->size = size;
        }

        ffStrbufSubstrBefore(path, baseLen);
    }
    return NULL;
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/dev/disk");
    if (!dir) {
        return "opendir(/dev/disk) failed";
    }

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateA(64);
    ffStrbufAppendS(&path, "/dev/disk/");
    uint32_t baseLen = path.length;

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.' || ffStrEquals(entry->d_name, "virtual")) {
            continue;
        }
        ffStrbufAppendS(&path, entry->d_name);
        searchRawDeviceFile(&path, entry->d_name, result, options);
        ffStrbufSubstrBefore(&path, baseLen);
    }

    return NULL;
}
