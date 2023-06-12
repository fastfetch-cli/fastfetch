#pragma once

#ifndef FF_INCLUDED_detection_vulkan
#define FF_INCLUDED_detection_vulkan

#include "fastfetch.h"

typedef struct FFVulkanResult
{
    FFstrbuf driver;
    FFstrbuf apiVersion;
    FFstrbuf conformanceVersion;
    FFlist gpus; //List of FFGPUResult, see detection/gpu/gpu.h
    const char* error;
} FFVulkanResult;

FFVulkanResult* ffDetectVulkan(const FFinstance* instance);

#endif
