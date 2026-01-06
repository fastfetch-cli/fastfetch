#pragma once

#include "common/option.h"

typedef struct FFCursorOptions
{
    FFModuleArgs moduleArgs;
} FFCursorOptions;

static_assert(sizeof(FFCursorOptions) <= FF_OPTION_MAX_SIZE, "FFCursorOptions size exceeds maximum allowed size");
