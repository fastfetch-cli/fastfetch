#pragma once

#ifndef FF_INCLUDED_detection_gpu_gpu
#define FF_INCLUDED_detection_gpu_gpu

#include "fastfetch.h"

#define FF_GPU_TEMP_UNSET (0/0.0)
#define FF_GPU_CORE_COUNT_UNSET -1

#define FF_GPU_VENDOR_NAME_AMD "AMD"
#define FF_GPU_VENDOR_NAME_INTEL "Intel"
#define FF_GPU_VENDOR_NAME_NVIDIA "NVIDIA"

typedef struct FFGPUResult
{
    FFstrbuf vendor;
    FFstrbuf name;
    FFstrbuf driver;
    double temperature;
    int coreCount;
} FFGPUResult;

const FFlist* ffDetectGPU(const FFinstance* instance);

#endif
