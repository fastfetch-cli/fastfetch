#pragma once

#include "common/option.h"

typedef struct FFPowerAdapterOptions
{
    FFModuleArgs moduleArgs;
} FFPowerAdapterOptions;

static_assert(sizeof(FFPowerAdapterOptions) <= FF_OPTION_MAX_SIZE, "FFPowerAdapterOptions size exceeds maximum allowed size");
