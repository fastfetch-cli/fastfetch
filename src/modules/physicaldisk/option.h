#pragma once

#include "common/option.h"

typedef struct FFPhysicalDiskOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf namePrefix;
    bool temp;
    FFColorRangeConfig tempConfig;
} FFPhysicalDiskOptions;

static_assert(sizeof(FFPhysicalDiskOptions) <= FF_OPTION_MAX_SIZE, "FFPhysicalDiskOptions size exceeds maximum allowed size");
