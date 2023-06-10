#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFBatteryOptions
{
    const char* moduleName;
    FFModuleArgs moduleArgs;

    bool temp;

    #ifdef __linux__
        FFstrbuf dir;
    #endif
} FFBatteryOptions;
