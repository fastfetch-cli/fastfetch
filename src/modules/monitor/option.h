#pragma once

#include "common/option.h"

typedef struct FFMonitorOptions
{
    FFModuleArgs moduleArgs;
} FFMonitorOptions;

static_assert(sizeof(FFMonitorOptions) <= FF_OPTION_MAX_SIZE, "FFMonitorOptions size exceeds maximum allowed size");
