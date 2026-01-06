#pragma once

#include "util/option.h"

typedef struct FFVersionOptions
{
    FFModuleArgs moduleArgs;
} FFVersionOptions;

static_assert(sizeof(FFVersionOptions) <= FF_OPTION_MAX_SIZE, "FFVersionOptions size exceeds maximum allowed size");
