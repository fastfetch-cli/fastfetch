#pragma once

#include "common/option.h"

typedef struct FFOSOptions
{
    FFModuleArgs moduleArgs;
} FFOSOptions;

static_assert(sizeof(FFOSOptions) <= FF_OPTION_MAX_SIZE, "FFOSOptions size exceeds maximum allowed size");
