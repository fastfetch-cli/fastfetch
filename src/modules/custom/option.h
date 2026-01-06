#pragma once

#include "common/option.h"

typedef struct FFCustomOptions
{
    FFModuleArgs moduleArgs;
} FFCustomOptions;

static_assert(sizeof(FFCustomOptions) <= FF_OPTION_MAX_SIZE, "FFCustomOptions size exceeds maximum allowed size");
