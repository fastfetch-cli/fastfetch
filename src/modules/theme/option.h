#pragma once

#include "common/option.h"

typedef struct FFThemeOptions
{
    FFModuleArgs moduleArgs;
} FFThemeOptions;

static_assert(sizeof(FFThemeOptions) <= FF_OPTION_MAX_SIZE, "FFThemeOptions size exceeds maximum allowed size");
