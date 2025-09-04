#pragma once

#include "fastfetch.h"
#include "modules/cpu/option.h"

#define FF_CPU_TEMP_UNSET (-DBL_MAX)

typedef struct FFCPUCore
{
    uint32_t freq;
    uint32_t count;
} FFCPUCore;

typedef struct FFCPUResult
{
    FFstrbuf name;
    FFstrbuf vendor;
    const char* march; // Microarchitecture

    uint16_t packages;
    uint16_t coresPhysical;
    uint16_t coresLogical;
    uint16_t coresOnline;

    uint32_t frequencyBase; // GHz
    uint32_t frequencyMax; // GHz

    FFCPUCore coreTypes[16]; // number of P cores, E cores, etc.

    double temperature;
} FFCPUResult;

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu);
const char* ffCPUAppleCodeToName(uint32_t code);
const char* ffCPUQualcommCodeToName(uint32_t code);
void ffCPUDetectByCpuid(FFCPUResult* cpu);
