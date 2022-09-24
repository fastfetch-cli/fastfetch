#pragma once

#ifndef FF_INCLUDED_detection_temps
#define FF_INCLUDED_detection_temps

#include "fastfetch.h"

typedef struct FFTempValue
{
    FFstrbuf name;
    FFstrbuf deviceClass;
    double value;
} FFTempValue;

typedef struct FFTempsResult
{
    FFlist values; //List of FFTempValue
} FFTempsResult;

const FFTempsResult* ffDetectTemps(const FFinstance* instance);

#endif
