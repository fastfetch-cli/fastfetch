#pragma once

#include "common/option.h"

typedef struct FFIconsOptions
{
    FFModuleArgs moduleArgs;
} FFIconsOptions;

static_assert(sizeof(FFIconsOptions) <= FF_OPTION_MAX_SIZE, "FFIconsOptions size exceeds maximum allowed size");
