#pragma once

#ifndef FF_INCLUDED_detection_brightness_brightness
#define FF_INCLUDED_detection_brightness_brightness

#include "fastfetch.h"
#include "util/FFlist.h"

typedef struct FFBrightnessResult
{
    FFstrbuf name;
    float value;
} FFBrightnessResult;

const char* ffDetectBrightness(FFlist* result); // list of FFBrightnessResult

#endif
