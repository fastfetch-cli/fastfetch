#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFMemoryOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFMemoryOptions;

static_assert(sizeof(FFMemoryOptions) <= FF_OPTION_MAX_SIZE, "FFMemoryOptions size exceeds maximum allowed size");
