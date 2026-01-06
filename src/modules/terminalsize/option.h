#pragma once

#include "util/option.h"

typedef struct FFTerminalSizeOptions
{
    FFModuleArgs moduleArgs;
} FFTerminalSizeOptions;

static_assert(sizeof(FFTerminalSizeOptions) <= FF_OPTION_MAX_SIZE, "FFTerminalSizeOptions size exceeds maximum allowed size");
