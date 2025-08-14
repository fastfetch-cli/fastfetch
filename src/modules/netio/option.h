#pragma once

#include "common/option.h"

typedef struct FFNetIOOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf namePrefix;
    uint32_t waitTime;
    bool defaultRouteOnly;
    bool detectTotal;
} FFNetIOOptions;

static_assert(sizeof(FFNetIOOptions) <= FF_OPTION_MAX_SIZE, "FFNetIOOptions size exceeds maximum allowed size");
