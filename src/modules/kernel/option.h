#pragma once

#include "common/option.h"

typedef struct FFKernelOptions
{
    FFModuleArgs moduleArgs;
} FFKernelOptions;

static_assert(sizeof(FFKernelOptions) <= FF_OPTION_MAX_SIZE, "FFKernelOptions size exceeds maximum allowed size");
