#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef enum FF_A_PACKED FFDiskVolumeType {
    FF_DISK_VOLUME_TYPE_NONE = 0,
    FF_DISK_VOLUME_TYPE_REGULAR_BIT = 1 << 0,
    FF_DISK_VOLUME_TYPE_HIDDEN_BIT = 1 << 1,
    FF_DISK_VOLUME_TYPE_EXTERNAL_BIT = 1 << 2,
    FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT = 1 << 3,
    FF_DISK_VOLUME_TYPE_UNKNOWN_BIT = 1 << 4,
    FF_DISK_VOLUME_TYPE_READONLY_BIT = 1 << 5,
    FF_DISK_VOLUME_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFDiskVolumeType;

typedef enum FF_A_PACKED FFDiskCalcType {
    FF_DISK_CALC_TYPE_FREE,
    FF_DISK_CALC_TYPE_AVAILABLE,
} FFDiskCalcType;

typedef struct FFDiskOptions {
    FFModuleArgs moduleArgs;

    FFstrbuf folders;
    FFstrbuf hideFolders;
    FFstrbuf hideFS;
    FFDiskVolumeType showTypes;
    FFDiskCalcType calcType;
    FFPercentageModuleConfig percent;
} FFDiskOptions;

static_assert(sizeof(FFDiskOptions) <= FF_OPTION_MAX_SIZE, "FFDiskOptions size exceeds maximum allowed size");
