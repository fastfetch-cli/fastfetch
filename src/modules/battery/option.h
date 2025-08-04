#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFBatteryOptions
{
    FFModuleArgs moduleArgs;

    bool temp;
    FFColorRangeConfig tempConfig;
    FFPercentageModuleConfig percent;

    #ifdef _WIN32
        bool useSetupApi;
    #endif
} FFBatteryOptions;

static_assert(sizeof(FFBatteryOptions) <= FF_OPTION_MAX_SIZE, "FFBatteryOptions size exceeds maximum allowed size");
