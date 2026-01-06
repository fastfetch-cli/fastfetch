#pragma once

#include "common/option.h"

typedef struct FFLocaleOptions
{
    FFModuleArgs moduleArgs;
} FFLocaleOptions;

static_assert(sizeof(FFLocaleOptions) <= FF_OPTION_MAX_SIZE, "FFLocaleOptions size exceeds maximum allowed size");
