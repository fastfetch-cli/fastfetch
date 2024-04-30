#include "disk.h"

bool ffDiskMatchMountpoint(FFDiskOptions* options, const char* mountpoint)
{
    #ifdef _WIN32
    const char separator = ';';
    #else
    const char separator = ':';
    #endif

    uint32_t mountpointLength = (uint32_t) strlen(mountpoint);

    uint32_t startIndex = 0;
    while(startIndex < options->folders.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&options->folders, startIndex, separator);

        uint32_t folderLength = colonIndex - startIndex;
        if (folderLength == mountpointLength && memcmp(options->folders.chars + startIndex, mountpoint, mountpointLength) == 0)
            return true;

        startIndex = colonIndex + 1;
    }

    return false;
}

static int compareDisks(const FFDisk* disk1, const FFDisk* disk2)
{
    return ffStrbufComp(&disk1->mountpoint, &disk2->mountpoint);
}

const char* ffDetectDisks(FFDiskOptions* options, FFlist* disks)
{
    const char* error = ffDetectDisksImpl(options, disks);

    if (error) return error;
    if (disks->length == 0) return "No disks found";

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
