#pragma once

#include "common/option.h"
#include "util/FFlist.h"

typedef struct FFMouseOptions
{
    FFModuleArgs moduleArgs;

    FFlist ignores; // List of FFstrbuf
} FFMouseOptions;

static_assert(sizeof(FFMouseOptions) <= FF_OPTION_MAX_SIZE, "FFMouseOptions size exceeds maximum allowed size");
