#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFZpoolOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFZpoolOptions;

static_assert(sizeof(FFZpoolOptions) <= FF_OPTION_MAX_SIZE, "FFZpoolOptions size exceeds maximum allowed size");
