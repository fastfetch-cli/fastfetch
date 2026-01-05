#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFSwapOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
    bool separate;
} FFSwapOptions;

static_assert(sizeof(FFSwapOptions) <= FF_OPTION_MAX_SIZE, "FFSwapOptions size exceeds maximum allowed size");
