#pragma once

#include "common/option.h"

typedef struct FFLoadavgOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
    uint8_t ndigits;
    bool compact;
} FFLoadavgOptions;

static_assert(sizeof(FFLoadavgOptions) <= FF_OPTION_MAX_SIZE, "FFLoadavgOptions size exceeds maximum allowed size");
