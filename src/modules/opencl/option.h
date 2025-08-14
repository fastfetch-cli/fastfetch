#pragma once

#include "common/option.h"

typedef struct FFOpenCLOptions
{
    FFModuleArgs moduleArgs;
} FFOpenCLOptions;

static_assert(sizeof(FFOpenCLOptions) <= FF_OPTION_MAX_SIZE, "FFOpenCLOptions size exceeds maximum allowed size");
