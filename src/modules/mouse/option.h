#pragma once

#include "common/option.h"

typedef struct FFMouseOptions
{
    FFModuleArgs moduleArgs;
} FFMouseOptions;

static_assert(sizeof(FFMouseOptions) <= FF_OPTION_MAX_SIZE, "FFMouseOptions size exceeds maximum allowed size");
