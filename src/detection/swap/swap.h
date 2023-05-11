#pragma once

#ifndef FF_INCLUDED_detection_swap_swap
#define FF_INCLUDED_detection_swap_swap

#include "fastfetch.h"

typedef struct FFSwapResult
{
    uint64_t bytesUsed;
    uint64_t bytesTotal;
} FFSwapResult;

const char* ffDetectSwap(FFSwapResult* result);

#endif
