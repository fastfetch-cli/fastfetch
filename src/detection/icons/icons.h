#pragma once

#include "fastfetch.h"

typedef struct FFIconsResult
{
    FFstrbuf icons1;
    FFstrbuf icons2;
} FFIconsResult;


const char* ffDetectIcons(FFIconsResult* result);
