#pragma once

#ifndef FF_INCLUDED_detection_memory_memory
#define FF_INCLUDED_detection_memory_memory

#include "fastfetch.h"

typedef struct FFMemoryResult
{
    uint64_t bytesUsed;
    uint64_t bytesTotal;
    uint8_t percentage;
} FFMemoryResult;

const FFMemoryResult* ffDetectMemory();

#endif
