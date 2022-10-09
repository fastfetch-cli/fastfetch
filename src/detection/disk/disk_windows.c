#include "disk.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static void detectDrive(const char* folderPath, uint32_t pathLen, FFDiskResult* result)
{
    ffStrbufInitNS(&result->path, pathLen, folderPath);
    result->removable = false; //To be set at other place
    result->files = 0; //Unsupported

    //According to https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getdiskfreespaceexa
    //GetDiskFreeSpaceExA does not have to specify the root directory on a disk. The function accepts any directory on a disk.
    uint64_t freeBytes;
    if(GetDiskFreeSpaceExA(folderPath, NULL, (PULARGE_INTEGER)&result->total, (PULARGE_INTEGER)&freeBytes)) {
        result->used = result->total - freeBytes;
        ffStrbufInit(&result->error);
    }
    else
        ffStrbufInitS(&result->error, "GetDiskFreeSpaceExA() failed");
}

const char* ffDiskAutodetectFolders(FFinstance* instance, FFlist* folders)
{
    FF_UNUSED(instance);

    char buf[128]; // "C:\\\0D:\\\0"
    uint32_t length = GetLogicalDriveStringsA(sizeof(buf) / sizeof(*buf), buf);

    for(size_t i = 0; i < length;)
    {
        const char* drive = buf + i;
        uint32_t driveLen = (uint32_t) strlen(drive);
        i += driveLen + 1;

        bool removable = GetDriveTypeA(drive) == DRIVE_REMOVABLE;
        if(removable && !instance->config.diskRemovable)
            continue;

        FFDiskResult* folder = (FFDiskResult*)ffListAdd(folders);
        detectDrive(drive, driveLen, folder);
        folder->removable = removable;
    }

    return NULL;
}

bool ffDiskDetectDiskFolders(FFinstance* instance, FFlist* folders)
{
    ffStrbufTrim(&instance->config.diskFolders, ';');
    if(instance->config.diskFolders.length == 0)
        return false;

    uint32_t startIndex = 0;
    while(startIndex < instance->config.diskFolders.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&instance->config.diskFolders, startIndex, ';');
        instance->config.diskFolders.chars[colonIndex] = '\0';

        detectDrive(instance->config.diskFolders.chars + startIndex, colonIndex - startIndex, (FFDiskResult*)ffListAdd(folders));

        startIndex = colonIndex + 1;
    }

    return true;
}
