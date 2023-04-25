#pragma once

#ifndef FF_INCLUDED_detection_opencl_opencl
#define FF_INCLUDED_detection_opencl_opencl

#include "fastfetch.h"

typedef struct FFOpenCLResult
{
    FFstrbuf version;
    FFstrbuf device;
    FFstrbuf vendor;
} FFOpenCLResult;

const char* ffDetectOpenCL(FFinstance* instance, FFOpenCLResult* result);

#endif
