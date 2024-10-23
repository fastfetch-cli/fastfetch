#include "disk.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <sys/mntent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <sys/mnttab.h>

static bool isPhysicalDevice(const struct mnttab* device)
{
    //Always show the root path
    if(ffStrEquals(device->mnt_mountp, "/"))
        return true;

    if(ffStrEquals(device->mnt_special, "none"))
        return false;

    //ZFS pool
    if(ffStrEquals(device->mnt_fstype, "zfs"))
        return true;

    //Pseudo filesystems don't have a device in /dev
    if(!ffStrStartsWith(device->mnt_special, "/dev/"))
        return false;

    struct stat deviceStat;
    if(stat(device->mnt_special, &deviceStat) != 0)
        return false;

    //Ignore all devices that are not block devices
    if(!S_ISBLK(deviceStat.st_mode))
        return false;

    return true;
}

static bool isSubvolume(const FFlist* disks, FFDisk* currentDisk)
{
    if(ffStrbufEqualS(&currentDisk->filesystem, "zfs"))
    {
        //ZFS subvolumes
        uint32_t index = ffStrbufFirstIndexC(&currentDisk->mountFrom, '/');
        if (index == currentDisk->mountFrom.length)
            return false;

        FF_STRBUF_AUTO_DESTROY zpoolName = ffStrbufCreateNS(index, currentDisk->mountFrom.chars);
        for(uint32_t i = 0; i < disks->length - 1; i++)
        {
            const FFDisk* otherDevice = ffListGet(disks, i);
            if(ffStrbufEqualS(&otherDevice->filesystem, "zfs") && ffStrbufStartsWith(&otherDevice->mountFrom, &zpoolName))
                return true;
        }

        return false;
    }
    else
    {
        //Filter all disks which device was already found. This catches BTRFS subvolumes.
        for(uint32_t i = 0; i < disks->length - 1; i++)
        {
            const FFDisk* otherDevice = ffListGet(disks, i);

            if(ffStrbufEqual(&currentDisk->mountFrom, &otherDevice->mountFrom))
                return true;
        }
    }

    return false;
}

static void detectType(const FFlist* disks, FFDisk* currentDisk, struct mnttab* device)
{
    if(hasmntopt(device, MNTOPT_NOBROWSE))
        currentDisk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(isSubvolume(disks, currentDisk))
        currentDisk->type = FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
    else
        currentDisk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;
    if (hasmntopt(device, MNTOPT_RO))
        currentDisk->type |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
}

static void detectStats(FFDisk* disk)
{
    struct statvfs fs;
    if(statvfs(disk->mountpoint.chars, &fs) != 0)
        memset(&fs, 0, sizeof(fs));

    disk->bytesTotal = fs.f_blocks * fs.f_frsize;
    disk->bytesFree = fs.f_bfree * fs.f_frsize;
    disk->bytesAvailable = fs.f_bavail * fs.f_frsize;
    disk->bytesUsed = 0; // To be filled in ./disk.c

    disk->filesTotal = (uint32_t) fs.f_files;
    disk->filesUsed = (uint32_t) (disk->filesTotal - fs.f_ffree);

    ffStrbufSetS(&disk->name, fs.f_fstr);

    disk->createTime = 0;
    struct stat deviceStat;
    if(stat(disk->mountpoint.chars, &deviceStat) == 0)
        disk->createTime = (uint64_t) deviceStat.st_ctim.tv_sec * 1000 + (uint64_t) deviceStat.st_ctim.tv_nsec / 1000000000;
}

const char* ffDetectDisksImpl(FFDiskOptions* options, FFlist* disks)
{
    FF_AUTO_CLOSE_FILE FILE* mountsFile = fopen(MNTTAB, "r");
    if(mountsFile == NULL)
        return "fopen(\"" MNTTAB "\", \"r\") == NULL";

    struct mnttab device;

    while (getmntent(mountsFile, &device) == 0)
    {
        if (__builtin_expect(options->folders.length, 0))
        {
            if (!ffDiskMatchMountpoint(options, device.mnt_mountp))
                continue;
        }
        else if(!isPhysicalDevice(&device))
            continue;

        //We have a valid device, add it to the list
        FFDisk* disk = ffListAdd(disks);
        disk->type = FF_DISK_VOLUME_TYPE_NONE;
        ffStrbufInitS(&disk->mountFrom, device.mnt_special);
        ffStrbufInitS(&disk->mountpoint, device.mnt_mountp);
        ffStrbufInitS(&disk->filesystem, device.mnt_fstype);
        ffStrbufInit(&disk->name);
        detectType(disks, disk, &device);
        detectStats(disk);
    }

    return NULL;
}
