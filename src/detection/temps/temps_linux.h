#pragma once

#include "fastfetch.h"

typedef struct FFTempValue
{
    FFstrbuf name;
    double value;
} FFTempValue;

const FFlist* /* List of FFTempValue */ ffDetectTemps();
