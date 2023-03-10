#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFDisplayCompactType
{
    FF_DISPLAY_COMPACT_TYPE_NONE = 0,
    FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT = 1 << 0,
    FF_DISPLAY_COMPACT_TYPE_SCALED_BIT = 1 << 1,
} FFDisplayCompactType;

typedef struct FFDisplayOptions
{
    const char* moduleName;
    FFModuleArgs moduleArgs;

    FFDisplayCompactType compactType;
    bool detectName;
    bool preciseRefreshRate;
} FFDisplayOptions;
