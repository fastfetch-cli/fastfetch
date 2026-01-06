#pragma once

#include "common/option.h"

typedef struct FFBootmgrOptions
{
    FFModuleArgs moduleArgs;
} FFBootmgrOptions;

static_assert(sizeof(FFBootmgrOptions) <= FF_OPTION_MAX_SIZE, "FFBootmgrOptions size exceeds maximum allowed size");
