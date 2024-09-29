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

    uint32_t frequencyBase; // GHz
    uint32_t frequencyMax; // GHz

    FFCPUCore coreTypes[16]; // number of P cores, E cores, etc.

    double temperature;
} FFCPUResult;

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu);
const char* ffCPUAppleCodeToName(uint32_t code);
const char* ffCPUQualcommCodeToName(uint32_t code);

#if defined(__x86_64__) || defined(__i386__)

#include <cpuid.h>

// WARNING: CPUID may report frequencies of efficient cores
inline static const char* ffCPUDetectSpeedByCpuid(FFCPUResult* cpu)
{
    uint32_t base = 0, max = 0, bus = 0, unused = 0;
    if (!__get_cpuid(0x16, &base, &max, &bus, &unused))
        return "Unsupported instruction";

    // cpuid returns 0 MHz when hyper-v is enabled
    if (base) cpu->frequencyBase = base;
    if (max) cpu->frequencyMax = max;
    return NULL;
}

#else

inline static const char* ffCPUDetectSpeedByCpuid(FF_MAYBE_UNUSED FFCPUResult* cpu)
{
    return "Unsupported platform";
}

#endif
