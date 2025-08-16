#pragma once

#include "common/option.h"

typedef struct FFWMThemeOptions
{
    FFModuleArgs moduleArgs;
} FFWMThemeOptions;

static_assert(sizeof(FFWMThemeOptions) <= FF_OPTION_MAX_SIZE, "FFWMThemeOptions size exceeds maximum allowed size");
