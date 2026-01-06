#pragma once

#include "util/option.h"

typedef struct FFInitSystemOptions
{
    FFModuleArgs moduleArgs;
} FFInitSystemOptions;

static_assert(sizeof(FFInitSystemOptions) <= FF_OPTION_MAX_SIZE, "FFInitSystemOptions size exceeds maximum allowed size");
