#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFMediaOptions {
    FFModuleArgs moduleArgs;
    FFPercentageModuleConfig percent;
} FFMediaOptions;

static_assert(sizeof(FFMediaOptions) <= FF_OPTION_MAX_SIZE, "FFMediaOptions size exceeds maximum allowed size");
