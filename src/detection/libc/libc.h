#pragma once

#include "fastfetch.h"

typedef struct FFLibcResult
{
    const char* name;
    const char* version;
} FFLibcResult;

const char* ffDetectLibc(FFLibcResult* result);
