#pragma once

#include "common/option.h"

typedef struct FFDiskIOOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf namePrefix;
    uint32_t waitTime;
    bool detectTotal;
} FFDiskIOOptions;

static_assert(sizeof(FFDiskIOOptions) <= FF_OPTION_MAX_SIZE, "FFDiskIOOptions size exceeds maximum allowed size");
