#pragma once

#include "fastfetch.h"

typedef struct FFVulkanResult
{
    FFstrbuf driver;
    FFstrbuf apiVersion;
    FFstrbuf conformanceVersion;
    FFlist gpus; //List of FFGPUResult, see detection/gpu/gpu.h
    const char* error;
} FFVulkanResult;

FFVulkanResult* ffDetectVulkan();
