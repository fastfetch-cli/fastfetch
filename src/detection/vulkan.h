#pragma once

#ifndef FF_INCLUDED_detection_vulkan
#define FF_INCLUDED_detection_vulkan

#include "fastfetch.h"

#define FF_GPU_TEMP_UNSET (0/0.0)

typedef struct FFGPUResult
{
    FFstrbuf vendor;
    FFstrbuf device;
    FFstrbuf driver;
    double temperature;
} FFGPUResult;

typedef struct FFVulkanResult
{
    FFstrbuf driver;
    FFstrbuf apiVersion;
    FFstrbuf conformanceVersion;
    FFlist devices; //List of FFGPUResult
} FFVulkanResult;

const FFVulkanResult* ffDetectVulkan(const FFinstance* instance);

#endif
