#pragma once

#include "util/option.h"

typedef struct FFTerminalFontOptions
{
    FFModuleArgs moduleArgs;
} FFTerminalFontOptions;

static_assert(sizeof(FFTerminalFontOptions) <= FF_OPTION_MAX_SIZE, "FFTerminalFontOptions size exceeds maximum allowed size");
