#pragma once

#include "common/option.h"

typedef struct FFWeatherOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf location;
    FFstrbuf outputFormat;
    uint32_t timeout;
} FFWeatherOptions;

static_assert(sizeof(FFWeatherOptions) <= FF_OPTION_MAX_SIZE, "FFWeatherOptions size exceeds maximum allowed size");
