#pragma once

#include "fastfetch.h"

#define FF_CPU_TEMP_UNSET (0/0.0)

typedef struct FFCPUCore
{
    uint32_t freq;
    uint32_t count;
} FFCPUCore;

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

    double temperature;
} FFCPUResult;

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu);
const char* ffCPUAppleCodeToName(uint32_t code);
