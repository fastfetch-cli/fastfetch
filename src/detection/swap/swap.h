#pragma once

#include "fastfetch.h"
#include "modules/swap/option.h"

typedef struct FFSwapResult
{
    FFstrbuf name;
    uint64_t bytesUsed;
    uint64_t bytesTotal;
} FFSwapResult;

const char* ffDetectSwap(FFlist* result /* List of FFSwapResult */);
