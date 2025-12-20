#pragma once

#include "common/option.h"
#include "util/FFlist.h"

typedef struct FFGamepadOptions
{
    FFModuleArgs moduleArgs;

    FFlist ignores; // List of FFstrbuf
    FFPercentageModuleConfig percent;
} FFGamepadOptions;

static_assert(sizeof(FFGamepadOptions) <= FF_OPTION_MAX_SIZE, "FFGamepadOptions size exceeds maximum allowed size");
