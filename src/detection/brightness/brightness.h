#pragma once

#include "fastfetch.h"
#include "util/FFlist.h"

typedef struct FFBrightnessResult
{
    FFstrbuf name;
    double min, max, current;
    bool builtin;
} FFBrightnessResult;

const char* ffDetectBrightness(FFBrightnessOptions* options, FFlist* result); // list of FFBrightnessResult
