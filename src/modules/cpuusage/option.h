#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"
#include "common/percent.h"

typedef struct FFCPUUsageOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    bool separate;
    FFPercentConfig percent;
} FFCPUUsageOptions;
