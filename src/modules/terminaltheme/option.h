#pragma once

#include "common/option.h"

typedef struct FFTerminalThemeOptions
{
    FFModuleArgs moduleArgs;
} FFTerminalThemeOptions;

static_assert(sizeof(FFTerminalThemeOptions) <= FF_OPTION_MAX_SIZE, "FFTerminalThemeOptions size exceeds maximum allowed size");
