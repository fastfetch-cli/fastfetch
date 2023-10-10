#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFCPUUsageDisplayType {
    FF_CPUUSAGE_DISPLAY_TYPE_DEFAULT,
    FF_CPUUSAGE_DISPLAY_TYPE_SEPARATE,
} FFCPUUsageDisplayType;

typedef struct FFCPUUsageOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFCPUUsageDisplayType displayType;
} FFCPUUsageOptions;
