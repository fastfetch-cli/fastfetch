#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFWeatherOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFstrbuf location;
    FFstrbuf outputFormat;
    uint32_t timeout;
} FFWeatherOptions;
