#pragma once

#ifndef FF_INCLUDED_detection_temps_apple
#define FF_INCLUDED_detection_temps_apple

#include "fastfetch.h"
#include "util/FFlist.h"

typedef struct FFTempValue
{
    FFstrbuf name;
    FFstrbuf deviceClass;
    double value;
} FFTempValue;

enum FFTempType
{
    FF_TEMP_CPU_X64,
    FF_TEMP_CPU_M1X,
    FF_TEMP_CPU_M2X,

    FF_TEMP_GPU_INTEL,
    FF_TEMP_GPU_AMD,
    FF_TEMP_GPU_UNKNOWN,
    FF_TEMP_GPU_M1X,
    FF_TEMP_GPU_M2X,

    FF_TEMP_BATTERY,

    FF_TEMP_MEMORY,
};

const char *ffDetectCoreTemps(enum FFTempType type, double* result);

#endif
