#pragma once

#include "common/option.h"

typedef struct FFCPUCacheOptions
{
    FFModuleArgs moduleArgs;

    bool compact;
} FFCPUCacheOptions;

static_assert(sizeof(FFCPUCacheOptions) <= FF_OPTION_MAX_SIZE, "FFCPUCacheOptions size exceeds maximum allowed size");
