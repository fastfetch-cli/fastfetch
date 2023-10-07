#include "disk.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <sys/mount.h>

#ifdef __FreeBSD__
static void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(ffStrbufEqualS(&disk->filesystem, "zfs"))
    {
        disk->type = !ffStrbufStartsWithS(&disk->mountFrom, "zroot/") || ffStrbufStartsWithS(&disk->mountFrom, "zroot/ROOT/")
            ? FF_DISK_VOLUME_TYPE_REGULAR_BIT
            : FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
    }
    else if(ffStrbufStartsWithS(&disk->mountpoint, "/boot") || ffStrbufStartsWithS(&disk->mountpoint, "/efi"))
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(!(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;
}
#elif __APPLE__
#include "util/apple/cf_helpers.h"

#include <sys/attr.h>
#include <unistd.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>

struct VolAttrBuf {
    uint32_t       length;
    attrreference_t volNameRef;
    char            volNameSpace[MAXPATHLEN];
} __attribute__((aligned(4), packed));

void detectDiskType(FFDisk* disk) // Not thread safe
{
    if (!ffStrbufStartsWithS(&disk->mountFrom, "/dev/disk")) return;

    static uint8_t cache[100]; // disk_id => physical_type + 1
    const char* numStart = disk->mountFrom.chars + strlen("/dev/disk");
    char* numEnd = NULL;
    unsigned long diskId = strtoul(numStart, &numEnd, 10);
    if (numEnd == numStart || diskId >= 100)
        return;

    if (cache[diskId])
    {
        disk->physicalType = cache[diskId] - 1;
        return;
    }

    io_iterator_t iterator;
    char temp = *numEnd; *numEnd = '\0'; // Check for root disk directly
    *numEnd = temp;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, IOBSDNameMatching(MACH_PORT_NULL, 0, disk->mountFrom.chars + strlen("/dev/")), &iterator) == kIOReturnSuccess)
    {
        for (io_registry_entry_t registryEntry = IOIteratorNext(iterator); registryEntry; IORegistryEntryGetParentEntry(registryEntry, kIOServicePlane, &registryEntry))
        {
            FF_CFTYPE_AUTO_RELEASE CFDictionaryRef deviceCharacteristics = (CFDictionaryRef) IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOPropertyDeviceCharacteristicsKey), kCFAllocatorDefault, kNilOptions);
            if (!deviceCharacteristics)
                continue;

            CFStringRef diskType = (CFStringRef) CFDictionaryGetValue(deviceCharacteristics, CFSTR(kIOPropertyMediumTypeKey));
            if (diskType)
            {
                if (CFStringCompare(diskType, CFSTR(kIOPropertyMediumTypeSolidStateKey), 0) == 0)
                    disk->physicalType = FF_DISK_PHYSICAL_TYPE_SSD;
                else if (CFStringCompare(diskType, CFSTR(kIOPropertyMediumTypeRotationalKey), 0) == 0)
                    disk->physicalType = FF_DISK_PHYSICAL_TYPE_HDD;
            }
            break;
        }

        IOObjectRelease(iterator);
    }

    cache[diskId] = (uint8_t) (disk->physicalType + 1);
}

void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(fs->f_flags & MNT_DONTBROWSE)
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(fs->f_flags & MNT_REMOVABLE || !(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;

    struct VolAttrBuf attrBuf;
    if (getattrlist(disk->mountpoint.chars, &(struct attrlist) {
        .bitmapcount = ATTR_BIT_MAP_COUNT,
        .volattr = ATTR_VOL_INFO | ATTR_VOL_NAME,
    }, &attrBuf, sizeof(attrBuf), 0) == 0)
        ffStrbufInitNS(&disk->name, attrBuf.volNameRef.attr_length - 1 /* excluding '\0' */, attrBuf.volNameSpace);

    detectDiskType(disk);
}
#endif

const char* ffDetectDisksImpl(FFlist* disks)
{
    int size = getfsstat(NULL, 0, MNT_WAIT);

    if(size <= 0)
        return "getfsstat(NULL, 0, MNT_WAIT) failed";

    FF_AUTO_FREE struct statfs* buf = malloc(sizeof(*buf) * (unsigned) size);
    if(getfsstat(buf, (int) (sizeof(*buf) * (unsigned) size), MNT_NOWAIT) <= 0)
        return "getfsstat(buf, size, MNT_NOWAIT) failed";

    for(struct statfs* fs = buf; fs < buf + size; ++fs)
    {
        if(!ffStrStartsWith(fs->f_mntfromname, "/dev/") && !ffStrEquals(fs->f_fstypename, "zfs"))
            continue;

        #ifdef __FreeBSD__
        // f_bavail and f_ffree are signed on FreeBSD...
        if(fs->f_bavail < 0) fs->f_bavail = 0;
        if(fs->f_ffree < 0) fs->f_ffree = 0;
        #endif

        FFDisk* disk = ffListAdd(disks);
        disk->physicalType = FF_DISK_PHYSICAL_TYPE_UNKNOWN;

        disk->bytesTotal = fs->f_blocks * fs->f_bsize;
        disk->bytesFree = (uint64_t)fs->f_bfree * fs->f_bsize;
        disk->bytesAvailable = (uint64_t)fs->f_bavail * fs->f_bsize;
        disk->bytesUsed = 0; // To be filled in ./disk.c

        disk->filesTotal = (uint32_t) fs->f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - (uint64_t)fs->f_ffree);

        ffStrbufInitS(&disk->mountFrom, fs->f_mntfromname);
        ffStrbufInitS(&disk->mountpoint, fs->f_mntonname);
        ffStrbufInitS(&disk->filesystem, fs->f_fstypename);
        ffStrbufInit(&disk->name);
        disk->type = 0;
        detectFsInfo(fs, disk);

        if(fs->f_flags & MNT_RDONLY)
            disk->type |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
    }

    return NULL;
}
