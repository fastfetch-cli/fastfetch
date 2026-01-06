#pragma once

#include "common/option.h"

typedef struct FFHostOptions
{
    FFModuleArgs moduleArgs;
} FFHostOptions;

static_assert(sizeof(FFHostOptions) <= FF_OPTION_MAX_SIZE, "FFHostOptions size exceeds maximum allowed size");
