#pragma once

#ifndef FF_INCLUDED_detection_lm_lm
#define FF_INCLUDED_detection_lm_lm

#include "fastfetch.h"

typedef struct FFLMResult
{
    FFstrbuf service;
    FFstrbuf type;
    FFstrbuf version;
} FFLMResult;

const char* ffDetectLM(FFLMResult* result);

#endif
