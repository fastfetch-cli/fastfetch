#pragma once

#include "common/option.h"

typedef struct FFMediaOptions
{
    FFModuleArgs moduleArgs;
} FFMediaOptions;

static_assert(sizeof(FFMediaOptions) <= FF_OPTION_MAX_SIZE, "FFMediaOptions size exceeds maximum allowed size");
