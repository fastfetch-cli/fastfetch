#pragma once

#include "common/option.h"

typedef struct FFLMOptions
{
    FFModuleArgs moduleArgs;
} FFLMOptions;

static_assert(sizeof(FFLMOptions) <= FF_OPTION_MAX_SIZE, "FFLMOptions size exceeds maximum allowed size");
