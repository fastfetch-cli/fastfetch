#pragma once

#include "common/option.h"

typedef struct FFGamepadOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFGamepadOptions;

static_assert(sizeof(FFGamepadOptions) <= FF_OPTION_MAX_SIZE, "FFGamepadOptions size exceeds maximum allowed size");
