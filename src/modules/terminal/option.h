#pragma once

#include "common/option.h"

typedef struct FFTerminalOptions
{
    FFModuleArgs moduleArgs;
} FFTerminalOptions;

static_assert(sizeof(FFTerminalOptions) <= FF_OPTION_MAX_SIZE, "FFTerminalOptions size exceeds maximum allowed size");
