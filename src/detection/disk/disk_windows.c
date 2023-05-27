#include "disk.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <assert.h>

void ffDetectDisksImpl(FFDiskResult* disks)
{
    wchar_t buf[MAX_PATH + 1];
    uint32_t length = GetLogicalDriveStringsW(sizeof(buf) / sizeof(*buf), buf);
    assert(length < sizeof(buf) / sizeof(*buf));

    for(uint32_t i = 0; i < length; i++)
    {
        const wchar_t* mountpoint = buf + i;

        UINT driveType = GetDriveTypeW(mountpoint);
        if(driveType == DRIVE_NO_ROOT_DIR)
        {
            i += (uint32_t)wcslen(mountpoint);
            continue;
        }

        FFDisk* disk = ffListAdd(&disks->disks);
        ffStrbufInitWS(&disk->mountpoint, mountpoint);

        uint64_t bytesFree;
        if(!GetDiskFreeSpaceExW(mountpoint, NULL, (PULARGE_INTEGER)&disk->bytesTotal, (PULARGE_INTEGER)&bytesFree))
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

        ffStrbufInit(&disk->filesystem);
        ffStrbufInit(&disk->name);
        wchar_t diskName[MAX_PATH + 1], diskFileSystem[MAX_PATH + 1];

        //https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getvolumeinformationa#remarks
        UINT errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        BOOL result = GetVolumeInformationW(mountpoint,
            diskName, sizeof(diskName) / sizeof(*diskName), //Volume name
            NULL, //Serial number
            NULL, //Max component length
            NULL, //File system flags
            diskFileSystem, sizeof(diskFileSystem) / sizeof(*diskFileSystem)
        );
        SetErrorMode(errorMode);

        if(result)
        {
            ffStrbufSetWS(&disk->filesystem, diskFileSystem);
            ffStrbufSetWS(&disk->name, diskName);
        }

        //TODO: implement
        disk->filesUsed = 0;
        disk->filesTotal = 0;

        i += disk->mountpoint.length;
    }
}
