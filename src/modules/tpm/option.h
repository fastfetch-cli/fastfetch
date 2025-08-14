#pragma once

#include "common/option.h"

typedef struct FFTPMOptions
{
    FFModuleArgs moduleArgs;
} FFTPMOptions;

static_assert(sizeof(FFTPMOptions) <= FF_OPTION_MAX_SIZE, "FFTPMOptions size exceeds maximum allowed size");
