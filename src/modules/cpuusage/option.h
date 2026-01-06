#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFCPUUsageOptions
{
    FFModuleArgs moduleArgs;

    bool separate;
    FFPercentageModuleConfig percent;
    uint32_t waitTime; // in ms
} FFCPUUsageOptions;

static_assert(sizeof(FFCPUUsageOptions) <= FF_OPTION_MAX_SIZE, "FFCPUUsageOptions size exceeds maximum allowed size");
