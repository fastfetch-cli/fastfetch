#pragma once

#include "common/option.h"

typedef struct FFWifiOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFWifiOptions;

static_assert(sizeof(FFWifiOptions) <= FF_OPTION_MAX_SIZE, "FFWifiOptions size exceeds maximum allowed size");
