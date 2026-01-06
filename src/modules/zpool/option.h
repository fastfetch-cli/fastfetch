#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFZpoolOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFZpoolOptions;

static_assert(sizeof(FFZpoolOptions) <= FF_OPTION_MAX_SIZE, "FFZpoolOptions size exceeds maximum allowed size");
