#include "disk.h"
#include "detection/internal.h"

void ffDetectDisksImpl(FFDiskResult* disks);

static int compareDisks(const void* disk1, const void* disk2)
{
    return ffStrbufCompAlphabetically(&((const FFDisk*) disk1)->mountpoint, &((const FFDisk*) disk2)->mountpoint);
}

const FFDiskResult* ffDetectDisks()
{
    FF_DETECTION_INTERNAL_GUARD(FFDiskResult,
        ffStrbufInit(&result.error);
        ffListInitA(&result.disks, sizeof(FFDisk), 4);

        ffDetectDisksImpl(&result);

        if(result.disks.length == 0 && result.error.length == 0)
            ffStrbufAppendS(&result.error, "No disks found");

        for(uint32_t i = 0; i < result.disks.length; ++i)
        {
            FFDisk* disk = ffListGet(&result.disks, i);
            disk->bytesPercentage = (uint8_t) (((long double) disk->bytesUsed / (long double) disk->bytesTotal) * 100.0);
            disk->filesPercentage = (uint8_t) (((long double) disk->filesUsed / (long double) disk->filesTotal) * 100.0);
        }

        //We need to sort the disks, so that we can detect, which disk a path resides on
        // For example for /boot/efi/bootmgr we need to check /boot/efi before /boot
        //Note that we sort alphabetically here for a better ordering when printing the list,
        // so the check must be done in reverse order
        ffListSort(&result.disks, compareDisks);
    );
}
