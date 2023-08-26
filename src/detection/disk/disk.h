#pragma once

#ifndef FF_INCLUDED_detection_disk_disk
#define FF_INCLUDED_detection_disk_disk

#include "fastfetch.h"

typedef struct FFDisk
{
    FFstrbuf mountpoint;
    FFstrbuf filesystem;
    FFstrbuf name;
    FFDiskVolumeType type;

    uint64_t bytesUsed;
    uint64_t bytesTotal;

    uint32_t filesUsed;
    uint32_t filesTotal;
} FFDisk;

/**
 * Returns a List of FFDisk, sorted alphabetically by mountpoint.
 * If error is not set, disks contains at least one disk.
 */
const char* ffDetectDisks(FFlist* result /* list of FFDisk */);

#endif
