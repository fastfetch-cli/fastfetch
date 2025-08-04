#pragma once

#include "common/option.h"

typedef struct FFShellOptions
{
    FFModuleArgs moduleArgs;
} FFShellOptions;

static_assert(sizeof(FFShellOptions) <= FF_OPTION_MAX_SIZE, "FFShellOptions size exceeds maximum allowed size");
