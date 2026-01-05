#pragma once

#include "util/option.h"

typedef struct FFProcessesOptions
{
    FFModuleArgs moduleArgs;
} FFProcessesOptions;

static_assert(sizeof(FFProcessesOptions) <= FF_OPTION_MAX_SIZE, "FFProcessesOptions size exceeds maximum allowed size");
