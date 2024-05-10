#pragma once

#include "fastfetch.h"

typedef struct FFPhysicalMemoryResult
{
    uint64_t size; // B
    uint32_t maxSpeed; // MT/s
    uint32_t runningSpeed; // MT/s
    FFstrbuf type;
    FFstrbuf formFactor;
    FFstrbuf deviceLocator;
    FFstrbuf vendor;
    FFstrbuf serial;
} FFPhysicalMemoryResult;

const char* ffDetectPhysicalMemory(FFlist* result); // list of FFPhysicalMemoryResult
