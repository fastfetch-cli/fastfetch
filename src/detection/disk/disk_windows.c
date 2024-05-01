#include "disk.h"
#include "common/io/io.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <winioctl.h>

const char* ffDetectDisksImpl(FFDiskOptions* options, FFlist* disks)
{
    wchar_t buf[MAX_PATH + 1];
    uint32_t length = GetLogicalDriveStringsW(sizeof(buf) / sizeof(*buf), buf);
    if (length == 0 || length >= sizeof(buf) / sizeof(*buf))
        return "GetLogicalDriveStringsW(sizeof(buf) / sizeof(*buf), buf) failed";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    // For cross-platform portability; used by `presets/examples/13.jsonc`
    if (__builtin_expect(options->folders.length == 1 && options->folders.chars[0] == '/', 0))
    {
        wchar_t path[MAX_PATH + 1];
        GetSystemWindowsDirectoryW(path, sizeof(path) / sizeof(*path));
        ffStrbufSetF(&options->folders, "%c:\\", (char) path[0]);
    }

    for(uint32_t i = 0; i < length; i++)
    {
        wchar_t* mountpoint = buf + i;

        ffStrbufSetWS(&buffer, mountpoint);
        i += buffer.length;

        UINT driveType = GetDriveTypeW(mountpoint);

        if (__builtin_expect((long) options->folders.length, 0))
        {
            if (!ffDiskMatchMountpoint(options, buffer.chars))
                continue;
        }
        else if(driveType == DRIVE_NO_ROOT_DIR)
            continue;

        FFDisk* disk = ffListAdd(disks);

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

        WIN32_FILE_ATTRIBUTE_DATA data;
        if(GetFileAttributesExW(mountpoint, GetFileExInfoStandard, &data) && data.ftCreationTime.dwHighDateTime > 0)
            disk->createTime = (*(uint64_t*) &data.ftCreationTime - 116444736000000000ull) / 10000ull;
        else
            disk->createTime = 0;

        ffStrbufInitMove(&disk->mountpoint, &buffer);
        if (mountpoint[2] == L'\\' && mountpoint[3] == L'\0')
        {
            wchar_t volumeName[MAX_PATH + 1];
            mountpoint[2] = L'\0';
            if(QueryDosDeviceW(mountpoint, volumeName, sizeof(volumeName) / sizeof(*volumeName)))
                ffStrbufInitWS(&disk->mountFrom, volumeName);
            else
                ffStrbufInit(&disk->mountFrom);
        }

        //Unsupported
        disk->filesUsed = 0;
        disk->filesTotal = 0;
    }

    return NULL;
}
