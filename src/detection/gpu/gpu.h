#pragma once

#ifndef FF_INCLUDED_detection_gpu_gpu
#define FF_INCLUDED_detection_gpu_gpu

#include "fastfetch.h"

#define FF_GPU_TEMP_UNSET (0/0.0)

typedef struct FFGPUResult
{
    FFstrbuf vendor;
    FFstrbuf name;
    FFstrbuf driver;
    double temperature;
} FFGPUResult;

const FFlist* ffDetectGPU(const FFinstance* instance);

#endif
