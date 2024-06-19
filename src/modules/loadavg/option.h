#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFLoadavgOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFColorRangeConfig percent;
    uint8_t ndigits;
    bool compact;
} FFLoadavgOptions;
