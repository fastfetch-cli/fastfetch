#pragma once

#include "common/option.h"

typedef struct FFDEOptions
{
    FFModuleArgs moduleArgs;

    bool slowVersionDetection;
} FFDEOptions;

static_assert(sizeof(FFDEOptions) <= FF_OPTION_MAX_SIZE, "FFDEOptions size exceeds maximum allowed size");
