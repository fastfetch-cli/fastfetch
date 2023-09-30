#include "disk.h"
#include "common/io/io.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <winioctl.h>
#include <assert.h>

const char* ffDetectDisksImpl(FFlist* disks)
{
    wchar_t buf[MAX_PATH + 1];
    uint32_t length = GetLogicalDriveStringsW(sizeof(buf) / sizeof(*buf), buf);
    if (length == 0 || length >= sizeof(buf) / sizeof(*buf))
        return "GetLogicalDriveStringsW(sizeof(buf) / sizeof(*buf), buf) failed";

    for(uint32_t i = 0; i < length; i++)
    {
        const wchar_t* mountpoint = buf + i;

        UINT driveType = GetDriveTypeW(mountpoint);
        if(driveType == DRIVE_NO_ROOT_DIR)
        {
            i += (uint32_t)wcslen(mountpoint);
            continue;
        }

        FFDisk* disk = ffListAdd(disks);
        ffStrbufInitWS(&disk->mountpoint, mountpoint);

        wchar_t volumeName[64];

        disk->physicalType = FF_DISK_PHYSICAL_TYPE_UNKNOWN;
        if(mountpoint[1] == ':')
        {
            memcpy(volumeName, L"\\\\.\\ :", sizeof(L"\\\\.\\ :"));
            volumeName[4] = mountpoint[0];
            FF_AUTO_CLOSE_FD HANDLE handle = CreateFileW(volumeName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (handle != INVALID_HANDLE_VALUE)
            {
                DEVICE_SEEK_PENALTY_DESCRIPTOR dspd = {};
                if(DeviceIoControl(
                    handle,
                    IOCTL_STORAGE_QUERY_PROPERTY,
                    &(STORAGE_PROPERTY_QUERY) {
                        .PropertyId = StorageDeviceSeekPenaltyProperty,
                        .QueryType = PropertyStandardQuery,
                    },
                    sizeof(STORAGE_PROPERTY_QUERY),
                    &dspd,
                    sizeof(dspd),
                    NULL,
                    NULL
                ))
                    disk->physicalType = dspd.IncursSeekPenalty ? FF_DISK_PHYSICAL_TYPE_HDD : FF_DISK_PHYSICAL_TYPE_SSD;
            }
        }

        if(GetVolumeNameForVolumeMountPointW(mountpoint, volumeName, sizeof(volumeName) / sizeof(*volumeName)))
        {
            ffStrbufInitWS(&disk->mountFrom, volumeName);
        }
        else
            ffStrbufInit(&disk->mountFrom);

        if(!GetDiskFreeSpaceExW(
            mountpoint,
            (PULARGE_INTEGER)&disk->bytesAvailable,
            (PULARGE_INTEGER)&disk->bytesTotal,
            (PULARGE_INTEGER)&disk->bytesFree
        ))
        {
            disk->bytesTotal = 0;
            disk->bytesFree = 0;
            disk->bytesAvailable = 0;
        }
        disk->bytesUsed = 0; // To be filled in ./disk.c

        if(driveType == DRIVE_REMOVABLE || driveType == DRIVE_REMOTE || driveType == DRIVE_CDROM)
            disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
        else if(driveType == DRIVE_FIXED)
            disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;
        else
            disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;

        ffStrbufInit(&disk->filesystem);
        ffStrbufInit(&disk->name);
        wchar_t diskName[MAX_PATH + 1], diskFileSystem[MAX_PATH + 1];

        //https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getvolumeinformationa#remarks
        UINT errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        DWORD diskFlags;
        BOOL result = GetVolumeInformationW(mountpoint,
            diskName, sizeof(diskName) / sizeof(*diskName), //Volume name
            NULL, //Serial number
            NULL, //Max component length
            &diskFlags, //File system flags
            diskFileSystem, sizeof(diskFileSystem) / sizeof(*diskFileSystem)
        );
        SetErrorMode(errorMode);

        if(result)
        {
            ffStrbufSetWS(&disk->filesystem, diskFileSystem);
            ffStrbufSetWS(&disk->name, diskName);
            if(diskFlags & FILE_READ_ONLY_VOLUME)
                disk->type |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
        }

        //Unsupported
        disk->filesUsed = 0;
        disk->filesTotal = 0;

        i += disk->mountpoint.length;
    }

    return NULL;
}
