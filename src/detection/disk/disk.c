#include "disk.h"

const char* ffDetectDisksImpl(FFlist* disks);

static int compareDisks(const FFDisk* disk1, const FFDisk* disk2)
{
    return ffStrbufComp(&disk1->mountpoint, &disk2->mountpoint);
}

const char* ffDetectDisks(FFDiskOptions* options, FFlist* disks)
{
    const char* error = ffDetectDisksImpl(disks);

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
