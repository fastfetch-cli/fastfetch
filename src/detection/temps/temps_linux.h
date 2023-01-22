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

typedef struct FFTempsResult
{
    FFlist values; //List of FFTempValue
} FFTempsResult;

const FFTempsResult* ffDetectTemps();

#endif
