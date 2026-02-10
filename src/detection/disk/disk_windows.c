#include "disk.h"
#include "common/io.h"
#include "common/windows/unicode.h"
#include "common/windows/nt.h"

#include <windows.h>
#include <winioctl.h>
#include <ntstatus.h>
#include <stdalign.h>

const char* ffDetectDisksImpl(FFDiskOptions* options, FFlist* disks)
{
    PROCESS_DEVICEMAP_INFORMATION_EX info = {};
    ULONG size = 0;
    if(!NT_SUCCESS(NtQueryInformationProcess(GetCurrentProcess(), ProcessDeviceMap, &info, sizeof(info), &size)))
        return "NtQueryInformationProcess(ProcessDeviceMap) failed";

    // For cross-platform portability; used by `presets/examples/13.jsonc`
    if (options->folders.length == 1 && options->folders.chars[0] == '/')
    {
        wchar_t path[MAX_PATH + 1];
        GetSystemWindowsDirectoryW(path, ARRAY_SIZE(path));
        options->folders.chars[0] = (char) path[0];
        ffStrbufAppendS(&options->folders, ":\\");
    }

    wchar_t mountpointW[] = L"X:\\";
    char mountpointA[] = "X:\\";

    for (wchar_t i = L'A'; i <= L'Z'; i++)
    {
        if (!(info.Query.DriveMap & (1 << (i - L'A'))))
            continue;
        mountpointW[0] = i;
        mountpointA[0] = (char) i;

        UINT driveType = info.Query.DriveType[i - L'A'];

        if (__builtin_expect((long) options->folders.length, 0))
        {
            if (!ffStrbufSeparatedContainNS(&options->folders, 3, mountpointA, FF_DISK_FOLDER_SEPARATOR))
                continue;
        }
        else if(driveType == DRIVE_NO_ROOT_DIR)
            continue;

        if (options->hideFolders.length && ffStrbufSeparatedContainNS(&options->hideFolders, 3, mountpointA, FF_DISK_FOLDER_SEPARATOR))
            continue;

        FF_AUTO_CLOSE_FD HANDLE handle = CreateFileW(mountpointW, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (handle == INVALID_HANDLE_VALUE)
            continue;

        IO_STATUS_BLOCK iosb;

        alignas(FILE_FS_ATTRIBUTE_INFORMATION) uint8_t bufFsAttr[1024];
        FILE_FS_ATTRIBUTE_INFORMATION* fsAttr = NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, bufFsAttr, sizeof(bufFsAttr), FileFsAttributeInformation))
            ? (FILE_FS_ATTRIBUTE_INFORMATION*) bufFsAttr
            : NULL;

        FF_STRBUF_AUTO_DESTROY diskFileSystemBuf = ffStrbufCreate();
        if (fsAttr)
        {
            ffStrbufSetNWS(&diskFileSystemBuf, fsAttr->FileSystemNameLength / sizeof(WCHAR), fsAttr->FileSystemName);
            if (options->hideFS.length && ffStrbufSeparatedContain(&options->hideFS, &diskFileSystemBuf, ':'))
                continue;
        }

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
        ffStrbufInitNS(&disk->mountpoint, 3, mountpointA);
        ffStrbufInit(&disk->mountFrom);
        disk->type = driveType == DRIVE_REMOVABLE || driveType == DRIVE_REMOTE || driveType == DRIVE_CDROM
            ? FF_DISK_VOLUME_TYPE_EXTERNAL_BIT
            : driveType == DRIVE_FIXED
                ? FF_DISK_VOLUME_TYPE_REGULAR_BIT
                : FF_DISK_VOLUME_TYPE_HIDDEN_BIT;

        {
            wchar_t volumeName[MAX_PATH + 1];
            mountpointW[2] = L'\0';
            if(QueryDosDeviceW(mountpointW, volumeName, ARRAY_SIZE(volumeName)))
                ffStrbufSetWS(&disk->mountFrom, volumeName);
            mountpointW[2] = L'\\';
        }

        alignas(FILE_FS_VOLUME_INFORMATION) uint8_t bufFsVolume[1024];
        FILE_FS_VOLUME_INFORMATION* fsVolume = NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, bufFsVolume, sizeof(bufFsVolume), FileFsVolumeInformation))
            ? (FILE_FS_VOLUME_INFORMATION*) bufFsVolume
            : NULL;

        if (fsVolume)
        {
            if (fsVolume->VolumeLabelLength > 0)
                ffStrbufSetNWS(&disk->name, fsVolume->VolumeLabelLength / sizeof(WCHAR), fsVolume->VolumeLabel);
            if (fsVolume->VolumeCreationTime.QuadPart)
                disk->createTime = ((uint64_t) fsVolume->VolumeCreationTime.QuadPart - 116444736000000000ull) / 10000ull;
        }

        if (fsAttr)
        {
            ffStrbufInitMove(&disk->filesystem, &diskFileSystemBuf);
            if(fsAttr->FileSystemAttributes & FILE_READ_ONLY_VOLUME)
                disk->type |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
        }

        FILE_FS_FULL_SIZE_INFORMATION fsFullSize;
        if (NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, &fsFullSize, sizeof(fsFullSize), FileFsFullSizeInformation)))
        {
            uint64_t units = fsFullSize.BytesPerSector * fsFullSize.SectorsPerAllocationUnit;
            disk->bytesTotal = (uint64_t) fsFullSize.TotalAllocationUnits.QuadPart * units;
            disk->bytesFree = (uint64_t) fsFullSize.ActualAvailableAllocationUnits.QuadPart * units;
            disk->bytesAvailable = (uint64_t) fsFullSize.CallerAvailableAllocationUnits.QuadPart * units;
        }
    }

    return NULL;
}
