#pragma once

#include "fastfetch.h"

typedef struct FFDotfileManagerResult
{
    FFstrbuf name;
} FFDotfileManagerResult;

const char* ffDetectDotfileManager(FFDotfileManagerResult* result);
