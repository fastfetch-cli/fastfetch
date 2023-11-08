#pragma once

#include "fastfetch.h"

typedef struct FFSwapResult
{
    uint64_t bytesUsed;
    uint64_t bytesTotal;
} FFSwapResult;

const char* ffDetectSwap(FFSwapResult* result);
