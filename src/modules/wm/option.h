#pragma once

#include "common/option.h"

typedef struct FFWMOptions
{
    FFModuleArgs moduleArgs;

    bool detectPlugin;
} FFWMOptions;

static_assert(sizeof(FFWMOptions) <= FF_OPTION_MAX_SIZE, "FFWMOptions size exceeds maximum allowed size");
