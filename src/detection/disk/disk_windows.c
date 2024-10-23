#include "disk.h"
#include "common/io/io.h"
#include "common/thread.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <winioctl.h>

static unsigned __stdcall testRemoteVolumeAccessible(void* mountpoint)
{
    FF_AUTO_CLOSE_FD HANDLE handle = CreateFileW(
        (wchar_t*) mountpoint,
        READ_CONTROL,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
    return 0;
}

const char* ffDetectDisksImpl(FFDiskOptions* options, FFlist* disks)
{
    wchar_t buf[MAX_PATH + 1];
    uint32_t length = GetLogicalDriveStringsW(ARRAY_SIZE(buf), buf);
    if (length == 0 || length >= ARRAY_SIZE(buf))
        return "GetLogicalDriveStringsW(ARRAY_SIZE(buf), buf) failed";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    // For cross-platform portability; used by `presets/examples/13.jsonc`
    if (__builtin_expect(options->folders.length == 1 && options->folders.chars[0] == '/', 0))
    {
        wchar_t path[MAX_PATH + 1];
        GetSystemWindowsDirectoryW(path, ARRAY_SIZE(path));
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

        disk->filesUsed = 0;
        disk->filesTotal = 0;
        disk->bytesTotal = 0;
        disk->bytesFree = 0;
        disk->bytesUsed = 0; // To be filled in ./disk.c
        disk->bytesAvailable = 0;
        disk->createTime = 0;
        ffStrbufInit(&disk->filesystem);
        ffStrbufInit(&disk->name);
        ffStrbufInitMove(&disk->mountpoint, &buffer);
        ffStrbufInit(&disk->mountFrom);
        disk->type = driveType == DRIVE_REMOVABLE || driveType == DRIVE_REMOTE || driveType == DRIVE_CDROM
            ? FF_DISK_VOLUME_TYPE_EXTERNAL_BIT
            : driveType == DRIVE_FIXED
                ? FF_DISK_VOLUME_TYPE_REGULAR_BIT
                : FF_DISK_VOLUME_TYPE_HIDDEN_BIT;

        if (mountpoint[2] == L'\\' && mountpoint[3] == L'\0')
        {
            wchar_t volumeName[MAX_PATH + 1];
            mountpoint[2] = L'\0';
            if(QueryDosDeviceW(mountpoint, volumeName, ARRAY_SIZE(volumeName)))
                ffStrbufSetWS(&disk->mountFrom, volumeName);
            mountpoint[2] = L'\\';
        }

        #ifdef FF_HAVE_THREADS
        if (driveType == DRIVE_REMOTE)
        {
            FFThreadType thread = ffThreadCreate(testRemoteVolumeAccessible, mountpoint);
            if (!ffThreadJoin(thread, 500))
                continue;
        }
        #endif

        GetDiskFreeSpaceExW(
            mountpoint,
            (PULARGE_INTEGER)&disk->bytesAvailable,
            (PULARGE_INTEGER)&disk->bytesTotal,
            (PULARGE_INTEGER)&disk->bytesFree
        );

        wchar_t diskName[MAX_PATH + 1], diskFileSystem[MAX_PATH + 1];

        //https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getvolumeinformationa#remarks
        UINT errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        DWORD diskFlags;
        BOOL result = GetVolumeInformationW(mountpoint,
            diskName, ARRAY_SIZE(diskName), //Volume name
            NULL, //Serial number
            NULL, //Max component length
            &diskFlags, //File system flags
            diskFileSystem, ARRAY_SIZE(diskFileSystem)
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
    }

    return NULL;
}
