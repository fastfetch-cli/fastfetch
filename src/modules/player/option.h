#pragma once

#include "common/option.h"

typedef struct FFPlayerOptions
{
    FFModuleArgs moduleArgs;
} FFPlayerOptions;

static_assert(sizeof(FFPlayerOptions) <= FF_OPTION_MAX_SIZE, "FFPlayerOptions size exceeds maximum allowed size");
