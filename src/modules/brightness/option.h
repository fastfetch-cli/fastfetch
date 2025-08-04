#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFBrightnessOptions
{
    FFModuleArgs moduleArgs;

    uint32_t ddcciSleep; // ms
    FFPercentageModuleConfig percent;
    bool compact;
} FFBrightnessOptions;

static_assert(sizeof(FFBrightnessOptions) <= FF_OPTION_MAX_SIZE, "FFBrightnessOptions size exceeds maximum allowed size");
