#pragma once

#include "fastfetch.h"

#define FF_CPU_TEMP_UNSET (0/0.0)

typedef struct FFCPUCore
{
    uint32_t freq;
    uint32_t count;
} FFCPUCore;

typedef enum FFCPUCacheType
{
    FF_CPU_CACHE_TYPE_UNIFIED = 0,
    FF_CPU_CACHE_TYPE_INSTRUCTION = 1,
    FF_CPU_CACHE_TYPE_DATA = 2,
    FF_CPU_CACHE_TYPE_TRACE = 3,
} FFCPUCacheType;

typedef struct FFCPUCache
{
    uint32_t size;
    uint32_t num;
    FFCPUCacheType type;
} FFCPUCache;

typedef struct FFCPUResult
{
    FFstrbuf name;
    FFstrbuf vendor;

    uint16_t coresPhysical;
    uint16_t coresLogical;
    uint16_t coresOnline;

    double frequencyBase; // GHz
    double frequencyMax; // GHz
    double frequencyMin; // GHz
    double frequencyBiosLimit; // GHz

    FFCPUCore coreTypes[16]; // number of P cores, E cores, etc.
    FFlist caches[3]; // L1, L2, L3

    double temperature;
} FFCPUResult;

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu);
const char* ffCPUAppleCodeToName(uint32_t code);
