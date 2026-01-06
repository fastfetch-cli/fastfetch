#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFPhysicalMemoryOptions
{
    FFModuleArgs moduleArgs;
} FFPhysicalMemoryOptions;

static_assert(sizeof(FFPhysicalMemoryOptions) <= FF_OPTION_MAX_SIZE, "FFPhysicalMemoryOptions size exceeds maximum allowed size");
