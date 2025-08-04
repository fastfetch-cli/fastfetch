#pragma once

#include "common/option.h"

typedef struct FFCPUOptions
{
    FFModuleArgs moduleArgs;

    bool temp;
    FFColorRangeConfig tempConfig;
    bool showPeCoreCount;
} FFCPUOptions;

static_assert(sizeof(FFCPUOptions) <= FF_OPTION_MAX_SIZE, "FFCPUOptions size exceeds maximum allowed size");
