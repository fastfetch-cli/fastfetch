#include "disk.h"

#include "util/stringUtils.h"

#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#ifdef __USE_LARGEFILE64
    #define stat stat64
    #define statvfs statvfs64
    #define dirent dirent64
    #define readdir readdir64
#endif

static bool isPhysicalDevice(const char* device)
{
    //DrvFs is a filesystem plugin to WSL that was designed to support interop between WSL and the Windows filesystem.
    if(strcmp(device, "drvfs") == 0)
        return true;

    //ZFS root pool. The format is rpool/<POOL_NAME>/<VOLUME_NAME>/<SUBVOLUME_NAME>
    if(strncmp(device, "rpool/", 6) == 0)
        return true;

    //Pseudo filesystems don't have a device in /dev
    const char* devPrefix = "/dev/";
    if(strncmp(device, devPrefix, strlen(devPrefix)) != 0)
        return false;

    //Skip /dev/ prefix
    device += strlen(devPrefix);

    if(
        strncmp(device, "loop", 4) == 0 || //Ignore loop devices
        strncmp(device, "ram", 3) == 0 ||  //Ignore ram devices
        strncmp(device, "fd", 2) == 0      //Ignore fd devices
    ) return false;

    return true;
}

static void appendNextEntry(FFstrbuf* buffer, char** source)
{
    //Read the current entry into the buffer
    while(**source != '\0' && !isspace(**source))
    {
        //After a backslash the next 3 characters are octal ascii codes
        if(**source == '\\' && strnlen(*source, 4) == 4)
        {
            char octal[4] = {0};
            strncpy(octal, *source + 1, 3);

            long value = strtol(octal, NULL, 8); //Returns 0 on error, so no need to check endptr
            if(value > 0 && value < CHAR_MAX)
            {
                ffStrbufAppendC(buffer, (char) value);
                *source += 4;
                continue;
            }
        }

        ffStrbufAppendC(buffer, **source);
        ++*source;
    }

    //Skip whitespace
    while(isspace(**source))
        ++*source;
}

static void detectNameFromPath(FFDisk* disk, const struct stat* deviceStat, FFstrbuf* basePath)
{
    DIR* dir = opendir(basePath->chars);
    if(dir == NULL)
        return;

    uint32_t basePathLength = basePath->length;

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(basePath, entry->d_name);

        struct stat entryStat;
        bool ret = stat(basePath->chars, &entryStat) == 0;

        ffStrbufSubstrBefore(basePath, basePathLength);

        if(!ret || deviceStat->st_ino != entryStat.st_ino)
            continue;

        ffStrbufAppendS(&disk->name, entry->d_name);
        break;
    }

    closedir(dir);
}

static void detectName(FFDisk* disk, const FFstrbuf* device)
{
    struct stat deviceStat;
    if(stat(device->chars, &deviceStat) != 0)
        return;

    FFstrbuf basePath;
    ffStrbufInit(&basePath);

    //Try partlabel first
    ffStrbufSetS(&basePath, "/dev/disk/by-partlabel/");
    detectNameFromPath(disk, &deviceStat, &basePath);

    //Try label second
    if(disk->name.length == 0)
    {
        ffStrbufSetS(&basePath, "/dev/disk/by-label/");
        detectNameFromPath(disk, &deviceStat, &basePath);
    }

    //Use the mountpoint as a last resort
    if(disk->name.length == 0)
        ffStrbufAppend(&disk->name, &disk->mountpoint);

    ffStrbufDestroy(&basePath);
}

#ifdef __ANDROID__

static void detectType(FF_MAYBE_UNUSED const FFlist* devices, FFDisk* currentDisk, FF_MAYBE_UNUSED const char* options)
{
    if(ffStrbufEqualS(&currentDisk->mountpoint, "/") || ffStrbufEqualS(&currentDisk->mountpoint, "/storage/emulated"))
        currentDisk->type = FF_DISK_TYPE_REGULAR;
    else if(ffStrbufStartsWithS(&currentDisk->mountpoint, "/mnt/media_rw/"))
        currentDisk->type = FF_DISK_TYPE_EXTERNAL;
    else
        currentDisk->type = FF_DISK_TYPE_HIDDEN;
}

#else

static bool isSubvolume(const FFlist* devices)
{
    const FFstrbuf* currentDevie = ffListGet(devices, devices->length - 1);

    //Filter all disks which device was already found. This catches BTRFS subvolumes.
    for(uint32_t i = 0; i < devices->length - 1; i++)
    {
        const FFstrbuf* otherDevice = ffListGet(devices, i);

        if(ffStrbufEqual(currentDevie, otherDevice))
            return true;
    }

    //ZFS subvolumes: rpool/<POOL_NAME>/<VOLUME_NAME>/<SUBVOLUME_NAME>.
    //Test if the third slash is present.
    if(strncmp(currentDevie->chars, "rpool/", 6) == 0 && ffStrHasNChars(currentDevie->chars, '/', 3))
        return true;

    return false;
}

static void detectType(const FFlist* devices, FFDisk* currentDisk, const char* options)
{
    if(isSubvolume(devices))
        currentDisk->type = FF_DISK_TYPE_SUBVOLUME;
    else if(strstr(options, "nosuid") != NULL || strstr(options, "nodev") != NULL)
        currentDisk->type = FF_DISK_TYPE_EXTERNAL;
    else if(ffStrbufStartsWithS(&currentDisk->mountpoint, "/boot") || ffStrbufStartsWithS(&currentDisk->mountpoint, "/efi"))
        currentDisk->type = FF_DISK_TYPE_HIDDEN;
    else
        currentDisk->type = FF_DISK_TYPE_REGULAR;
}

#endif

static void detectStats(FFDisk* disk)
{
    struct statvfs fs;
    if(statvfs(disk->mountpoint.chars, &fs) != 0)
        memset(&fs, 0, sizeof(struct statvfs)); //Set all values to 0, so our values get initialized to 0 too

    disk->bytesTotal = fs.f_blocks * fs.f_frsize;
    disk->bytesUsed = disk->bytesTotal - (fs.f_bavail * fs.f_frsize);

    disk->filesTotal = (uint32_t) fs.f_files;
    disk->filesUsed = (uint32_t) (disk->filesTotal - fs.f_ffree);
}

void ffDetectDisksImpl(FFDiskResult* disks)
{
    FILE* mountsFile = fopen("/proc/mounts", "r");
    if(mountsFile == NULL)
    {
        ffStrbufAppendS(&disks->error, "fopen(\"/proc/mounts\", \"r\") == NULL");
        return;
    }

    FFlist devices;
    ffListInit(&devices, sizeof(FFstrbuf));

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, mountsFile) != EOF)
    {
        if(!isPhysicalDevice(line))
            continue;

        //We have a valid device, add it to the list
        FFDisk* disk = ffListAdd(&disks->disks);

        //Format of the file: "<device> <mountpoint> <filesystem> <options> ..." (Same as fstab)
        char* currentPos = line;

        //detect device
        FFstrbuf* device = ffListAdd(&devices);
        ffStrbufInit(device);
        appendNextEntry(device, &currentPos);

        //detect mountpoint
        ffStrbufInit(&disk->mountpoint);
        appendNextEntry(&disk->mountpoint, &currentPos);

        //detect filesystem
        ffStrbufInit(&disk->filesystem);
        appendNextEntry(&disk->filesystem, &currentPos);

        //detect name
        ffStrbufInit(&disk->name);
        detectName(disk, device);

        //detect type
        detectType(&devices, disk, currentPos);

        //Detects stats
        detectStats(disk);
    }

    if(line != NULL)
        free(line);

    FF_LIST_FOR_EACH(FFstrbuf, device, devices)
        ffStrbufDestroy(device);
    ffListDestroy(&devices);

    fclose(mountsFile);
}
