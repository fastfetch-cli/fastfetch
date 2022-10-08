#pragma once

#ifndef FF_INCLUDED_detection_kernel_kernel
#define FF_INCLUDED_detection_kernel_kernel

#include "fastfetch.h"

typedef struct FFKernelResult
{
    FFstrbuf sysname;
    FFstrbuf release;
    FFstrbuf version;
    FFstrbuf error;
} FFKernelResult;

void ffDetectKernel(FFinstance* instance, FFKernelResult* result);

#endif
