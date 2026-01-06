#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFMemoryOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFMemoryOptions;

static_assert(sizeof(FFMemoryOptions) <= FF_OPTION_MAX_SIZE, "FFMemoryOptions size exceeds maximum allowed size");
