#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"
#include "common/percent.h"

typedef struct FFBrightnessOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    uint32_t ddcciSleep; // ms
    FFColorRangeConfig percent;
    bool compact;
} FFBrightnessOptions;
