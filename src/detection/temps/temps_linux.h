#pragma once

#include "fastfetch.h"

typedef struct FFTempValue
{
    FFstrbuf name;
    FFstrbuf deviceName;
    uint32_t deviceClass;
    double value;
} FFTempValue;

const FFlist* /* List of FFTempValue */ ffDetectTemps();
