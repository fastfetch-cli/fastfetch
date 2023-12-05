#pragma once

#include "fastfetch.h"

typedef struct FFTempValue
{
    FFstrbuf name;
    uint32_t deviceClass;
    double value;
} FFTempValue;

const FFlist* /* List of FFTempValue */ ffDetectTemps();
