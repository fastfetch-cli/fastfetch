#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFDiskType
{
    FF_DISK_TYPE_NONE = 0,
    FF_DISK_TYPE_REGULAR_BIT = 1 << 0,
    FF_DISK_TYPE_HIDDEN_BIT = 1 << 1,
    FF_DISK_TYPE_EXTERNAL_BIT = 1 << 2,
    FF_DISK_TYPE_SUBVOLUME_BIT = 1 << 3,
    FF_DISK_TYPE_UNKNOWN_BIT = 1 << 4,
} FFDiskType;

typedef struct FFDiskOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFstrbuf folders;
    FFDiskType showTypes;
} FFDiskOptions;
