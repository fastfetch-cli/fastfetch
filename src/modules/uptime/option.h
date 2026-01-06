#pragma once

#include "util/option.h"

typedef struct FFUptimeOptions
{
    FFModuleArgs moduleArgs;
} FFUptimeOptions;

static_assert(sizeof(FFUptimeOptions) <= FF_OPTION_MAX_SIZE, "FFUptimeOptions size exceeds maximum allowed size");
