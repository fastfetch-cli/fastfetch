#pragma once

#include "common/option.h"

typedef struct FFKeyboardOptions
{
    FFModuleArgs moduleArgs;
} FFKeyboardOptions;

static_assert(sizeof(FFKeyboardOptions) <= FF_OPTION_MAX_SIZE, "FFKeyboardOptions size exceeds maximum allowed size");
