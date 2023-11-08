#pragma once

#include "fastfetch.h"

typedef struct FFMemoryResult
{
    uint64_t bytesUsed;
    uint64_t bytesTotal;
} FFMemoryResult;

const char* ffDetectMemory(FFMemoryResult* result);
