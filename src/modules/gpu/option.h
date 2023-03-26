#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFGPUType
{
    FF_GPU_TYPE_UNKNOWN,
    FF_GPU_TYPE_INTEGRATED,
    FF_GPU_TYPE_DISCRETE,
} FFGPUType;

typedef struct FFGPUOptions
{
    const char* moduleName;
    FFModuleArgs moduleArgs;

    FFGPUType hideType;
    bool temp;
    bool forceVulkan;
} FFGPUOptions;
