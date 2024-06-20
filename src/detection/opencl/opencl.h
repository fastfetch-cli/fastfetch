#pragma once

#include "fastfetch.h"

typedef struct FFOpenCLResult
{
    FFstrbuf version;
    FFstrbuf name;
    FFstrbuf vendor;
    FFlist gpus; //List of FFGPUResult, see detection/gpu/gpu.h
} FFOpenCLResult;

const char* ffDetectOpenCL(FFOpenCLResult* result);
