#pragma once

#ifndef FF_INCLUDED_detection_temps_linux
#define FF_INCLUDED_detection_temps_linux

#include "fastfetch.h"

typedef struct FFTempValue
{
    FFstrbuf name;
    uint32_t deviceClass;
    double value;
} FFTempValue;

const FFlist* /* List of FFTempValue */ ffDetectTemps();

#endif
