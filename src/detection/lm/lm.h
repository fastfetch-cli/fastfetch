#pragma once

#include "fastfetch.h"

typedef struct FFLMResult
{
    FFstrbuf service;
    FFstrbuf type;
    FFstrbuf version;
} FFLMResult;

const char* ffDetectLM(FFLMResult* result);
