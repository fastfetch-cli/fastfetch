#pragma once

#include "common/option.h"

typedef struct FFPhysicalMemoryOptions {
    FFModuleArgs moduleArgs;
    bool showEmptySlots;
} FFPhysicalMemoryOptions;

static_assert(sizeof(FFPhysicalMemoryOptions) <= FF_OPTION_MAX_SIZE, "FFPhysicalMemoryOptions size exceeds maximum allowed size");
