#include "disk.h"

static int compareDisks(const FFDisk* disk1, const FFDisk* disk2)
{
    return ffStrbufComp(&disk1->mountpoint, &disk2->mountpoint);
}

const char* ffDetectDisks(FFDiskOptions* options, FFlist* disks)
{
    const char* error = ffDetectDisksImpl(options, disks);

    if (error) return error;
    if (disks->length == 0) return NULL;

    //We need to sort the disks, so that we can detect, which disk a path resides on
    // For example for /boot/efi/bootmgr we need to check /boot/efi before /boot
    //Note that we sort alphabetically here for a better ordering when printing the list,
    // so the check must be done in reverse order
    ffListSort(disks, (void*) compareDisks);
    FF_LIST_FOR_EACH(FFDisk, disk, *disks)
    {
        if(disk->bytesTotal == 0)
            disk->type |= FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
        else
        {
            disk->bytesUsed = disk->bytesTotal - (
                options->calcType == FF_DISK_CALC_TYPE_FREE ? disk->bytesFree : disk->bytesAvailable
            );
        }
    }

    return NULL;
}

#ifndef _WIN32
#include <fnmatch.h>

bool ffDiskMatchesFolderPatterns(FFstrbuf* folders, const char* path, char separator)
{
    uint32_t startIndex = 0;
    while(startIndex < folders->length)
    {
        uint32_t sepIndex = ffStrbufNextIndexC(folders, startIndex, separator);

        char savedSep = folders->chars[sepIndex]; // Can be '\0' if at end
        folders->chars[sepIndex] = '\0';

        bool matched = fnmatch(&folders->chars[startIndex], path, 0) == 0;
        folders->chars[sepIndex] = savedSep;

        if (matched) return true;

        startIndex = sepIndex + 1;
    }
    return false;
}
#endif
