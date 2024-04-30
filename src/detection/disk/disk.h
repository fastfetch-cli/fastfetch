#pragma once

#include "fastfetch.h"

typedef struct FFDisk
{
    FFstrbuf mountFrom;
    FFstrbuf mountpoint;
    FFstrbuf filesystem;
    FFstrbuf name;
    FFDiskVolumeType type;

    uint64_t bytesUsed;
    uint64_t bytesFree;
    uint64_t bytesAvailable;
    uint64_t bytesTotal;

    uint32_t filesUsed;
    uint32_t filesTotal;

    uint64_t createTime;
} FFDisk;

/**
 * Returns a List of FFDisk, sorted alphabetically by mountpoint.
 * If error is not set, disks contains at least one disk.
 */
const char* ffDetectDisks(FFDiskOptions* options, FFlist* disks /* list of FFDisk */);

const char* ffDetectDisksImpl(FFDiskOptions* options, FFlist* disks);
bool ffDiskMatchMountpoint(FFDiskOptions* options, const char* mountpoint);
