#include "disk.h"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

void ffDetectDisksImpl(FFDiskResult* disks)
{
    uint32_t length = GetLogicalDriveStringsA(0, NULL);
    if(length == 0)
    {
        ffStrbufAppendS(&disks->error, "GetLogicalDriveStringsA failed");
        return;
    }

    char* buf = malloc(length + 1);
    GetLogicalDriveStringsA(length, buf);

    for(uint32_t i = 0; i < length; i++)
    {
        const char* mountpoint = buf + i;

        UINT driveType = GetDriveTypeA(mountpoint);
        if(driveType == DRIVE_NO_ROOT_DIR)
        {
            i += strlen(mountpoint);
            continue;
        }

        FFDisk* disk = ffListAdd(&disks->disks);
        ffStrbufInitS(&disk->mountpoint, mountpoint);

        uint64_t bytesFree;
        if(!GetDiskFreeSpaceExA(mountpoint, NULL, (PULARGE_INTEGER)&disk->bytesTotal, (PULARGE_INTEGER)&bytesFree))
        {
            disk->bytesTotal = 0;
            bytesFree = 0;
        }
        disk->bytesUsed = disk->bytesTotal - bytesFree;

        if(driveType == DRIVE_REMOVABLE || driveType == DRIVE_REMOTE || driveType == DRIVE_CDROM)
            disk->type = FF_DISK_TYPE_EXTERNAL;
        else if(driveType == DRIVE_FIXED)
            disk->type = FF_DISK_TYPE_REGULAR;
        else
            disk->type = FF_DISK_TYPE_HIDDEN;

        ffStrbufInitA(&disk->filesystem, MAX_PATH + 1);
        GetVolumeInformationA(mountpoint
            NULL, 0, //Volume name
            NULL, //Serial number
            NULL, //Max component length
            NULL, //File system flags
            disk->filesystem.chars, ffStrbufGetFree(&disk->filesystem)
        );
        ffStrbufRecalculateLength(&disk->filesystem);

        //TODO: implement
        disk->filesUsed = 0;
        disk->filesTotal = 0;

        i += disk->mountpoint.length;
    }

    free(buff);
}
