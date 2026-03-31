#pragma once

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFPhysicalDiskType {
    FF_PHYSICALDISK_TYPE_NONE = 0,

    // If none is set, it's unknown
    FF_PHYSICALDISK_TYPE_HDD = 1 << 0,
    FF_PHYSICALDISK_TYPE_SSD = 1 << 1,
    FF_PHYSICALDISK_TYPE_VIRTUAL = 1 << 2,

    FF_PHYSICALDISK_TYPE_FIXED = 1 << 3,
    FF_PHYSICALDISK_TYPE_REMOVABLE = 1 << 4,

    FF_PHYSICALDISK_TYPE_READWRITE = 1 << 5,
    FF_PHYSICALDISK_TYPE_READONLY = 1 << 6,

    FF_PHYSICALDISK_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFPhysicalDiskType;
static_assert(sizeof(FFPhysicalDiskType) == sizeof(uint8_t), "");

typedef struct FFPhysicalDiskOptions {
    FFModuleArgs moduleArgs;

    FFstrbuf namePrefix;
    bool temp;
    FFColorRangeConfig tempConfig;
} FFPhysicalDiskOptions;

static_assert(sizeof(FFPhysicalDiskOptions) <= FF_OPTION_MAX_SIZE, "FFPhysicalDiskOptions size exceeds maximum allowed size");
