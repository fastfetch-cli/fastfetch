#pragma once

#include "fastfetch.h"

typedef struct FFPhysicalMemoryResult
{
    uint64_t size; // B
    uint32_t maxSpeed; // MT/s
    uint32_t runningSpeed; // MT/s
    FFstrbuf type;
    FFstrbuf formFactor;
    FFstrbuf locator;
    FFstrbuf partNumber;
    FFstrbuf vendor;
    FFstrbuf serial;
    bool ecc;
} FFPhysicalMemoryResult;

const char* ffDetectPhysicalMemory(FFlist* result); // list of FFPhysicalMemoryResult

void FFPhysicalMemoryUpdateVendorString(FFPhysicalMemoryResult* device);
