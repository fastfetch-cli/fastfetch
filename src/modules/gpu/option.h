#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"
#include "common/percent.h"

typedef enum __attribute__((__packed__)) FFGPUType
{
    FF_GPU_TYPE_UNKNOWN,
    FF_GPU_TYPE_INTEGRATED,
    FF_GPU_TYPE_DISCRETE,
} FFGPUType;

typedef enum __attribute__((__packed__)) FFGPUDetectionMethod
{
    FF_GPU_DETECTION_METHOD_AUTO,
    FF_GPU_DETECTION_METHOD_PCI,
    FF_GPU_DETECTION_METHOD_VULKAN,
    FF_GPU_DETECTION_METHOD_OPENCL,
    FF_GPU_DETECTION_METHOD_OPENGL,
} FFGPUDetectionMethod;

typedef struct FFGPUOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFGPUType hideType;
    FFGPUDetectionMethod detectionMethod;
    bool temp;
    bool driverSpecific;
    bool forceMethod;
    FFColorRangeConfig tempConfig;
    FFPercentageModuleConfig percent;
} FFGPUOptions;
