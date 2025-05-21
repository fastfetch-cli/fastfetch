#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"
#include "common/percent.h"

typedef enum __attribute__((__packed__)) FFDiskVolumeType
{
    FF_DISK_VOLUME_TYPE_NONE = 0,
    FF_DISK_VOLUME_TYPE_REGULAR_BIT = 1 << 0,
    FF_DISK_VOLUME_TYPE_HIDDEN_BIT = 1 << 1,
    FF_DISK_VOLUME_TYPE_EXTERNAL_BIT = 1 << 2,
    FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT = 1 << 3,
    FF_DISK_VOLUME_TYPE_UNKNOWN_BIT = 1 << 4,
    FF_DISK_VOLUME_TYPE_READONLY_BIT = 1 << 5,
    FF_DISK_VOLUME_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFDiskVolumeType;

typedef enum __attribute__((__packed__)) FFDiskCalcType
{
    FF_DISK_CALC_TYPE_FREE,
    FF_DISK_CALC_TYPE_AVAILABLE,
} FFDiskCalcType;

typedef struct FFDiskOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFstrbuf folders;
    FFstrbuf hideFolders;
    FFstrbuf hideFS;
    FFDiskVolumeType showTypes;
    FFDiskCalcType calcType;
    FFPercentageModuleConfig percent;
} FFDiskOptions;
