#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFPhysicalMemoryOptions
{
    FFModuleArgs moduleArgs;
} FFPhysicalMemoryOptions;

static_assert(sizeof(FFPhysicalMemoryOptions) <= FF_OPTION_MAX_SIZE, "FFPhysicalMemoryOptions size exceeds maximum allowed size");
