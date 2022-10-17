#include "disk.h"

#include <ctype.h>
#include <sys/statvfs.h>

void ffDetectDisksImpl(FFDiskResult* disks)
{
    FILE* mountsFile = fopen("/proc/mounts", "r");
    if(mountsFile == NULL)
    {
        ffStrbufAppendS(&disks->error, "fopen(\"/proc/mounts\", \"r\") == NULL");
        return;
    }

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, mountsFile) != EOF)
    {
        //Format of the file: "<device> <mountpoint> <filesystem> <options> ..." (Same as fstab)
        char* currentPos = line;

        //Non pseudo filesystems have their device in /dev/, we only add those
        if(strncasecmp(currentPos, "/dev/", 5) != 0)
            continue;

        //Skip /dev/
        currentPos += 5;

        //Don't show loop file systems
        if(strncasecmp(currentPos, "loop", 4) == 0)
            continue;

        FFDisk* disk = ffListAdd(&disks->disks);

        //Go to mountpoint
        while(!isspace(*currentPos) && *currentPos != '\0')
            ++currentPos;
        while(isspace(*currentPos))
            ++currentPos;

        ffStrbufInitA(&disk->mountpoint, 16);
        ffStrbufAppendSUntilC(&disk->mountpoint, currentPos, ' ');

        //Go to filesystem
        currentPos += disk->mountpoint.length;
        while(isspace(*currentPos))
            ++currentPos;

        ffStrbufInitA(&disk->filesystem, 16);
        ffStrbufAppendSUntilC(&disk->filesystem, currentPos, ' ');

        //Go to options, detect type
        currentPos += disk->filesystem.length;
        while(isspace(*currentPos))
            ++currentPos;

        if(strstr(currentPos, "nosuid") != NULL || strstr(currentPos, "nodev") != NULL)
            disk->type = FF_DISK_TYPE_EXTERNAL;
        else if(ffStrbufStartsWithS(&disk->mountpoint, "/boot") || ffStrbufStartsWithS(&disk->mountpoint, "/efi"))
            disk->type = FF_DISK_TYPE_HIDDEN;
        else
            disk->type = FF_DISK_TYPE_REGULAR;

        //Detects stats
        struct statvfs fs;
        if(statvfs(disk->mountpoint.chars, &fs) != 0)
            memset(&fs, 0, sizeof(struct statvfs)); //Set all values to 0, so our values get initialized to 0 too

        disk->bytesTotal = fs.f_blocks * fs.f_frsize;
        disk->bytesUsed = disk->bytesTotal - (fs.f_bavail * fs.f_frsize);

        disk->filesTotal = (uint32_t) fs.f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - fs.f_ffree);
    }

    if(line != NULL)
        free(line);

    fclose(mountsFile);
}
