#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFSwapOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
    bool separate;
} FFSwapOptions;

static_assert(sizeof(FFSwapOptions) <= FF_OPTION_MAX_SIZE, "FFSwapOptions size exceeds maximum allowed size");
