#pragma once

#include "fastfetch.h"
#include "modules/opencl/option.h"

typedef struct FFOpenCLResult
{
    FFstrbuf version;
    FFstrbuf name;
    FFstrbuf vendor;
    FFlist gpus; //List of FFGPUResult, see detection/gpu/gpu.h
    const char* error;
} FFOpenCLResult;

FFOpenCLResult* ffDetectOpenCL();
