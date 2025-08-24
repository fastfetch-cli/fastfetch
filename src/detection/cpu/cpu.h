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

#if defined(__x86_64__) || defined(__i386__)

#include <cpuid.h>

inline static void ffCPUDetectByCpuid(FFCPUResult* cpu)
{
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    if (__get_cpuid(0x16, &eax, &ebx, &ecx, &edx))
    {
        // WARNING: CPUID may report frequencies of efficient cores
        // cpuid returns 0 MHz when hypervisor is enabled
        if (eax) cpu->frequencyBase = eax;
        if (ebx) cpu->frequencyMax = ebx;
    }

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
    {
        // Feature tests (leaf1.ecx, leaf7.ebx)
        bool sse2     = (ecx & bit_SSE2) != 0;
        bool sse4_2   = (ecx & bit_SSE4_2) != 0;
        bool pclmul   = (ecx & bit_PCLMUL) != 0;
        bool popcnt   = (ecx & bit_POPCNT) != 0;
        bool fma      = (ecx & bit_FMA) != 0;
        bool osxsave  = (ecx & bit_OSXSAVE) != 0;

        unsigned int eax7 = 0, ebx7 = 0, ecx7 = 0, edx7 = 0;
        __get_cpuid_count(7, 0, &eax7, &ebx7, &ecx7, &edx7);

        bool avx2     = (ebx7 & bit_AVX2) != 0;
        bool bmi2     = (ebx7 & bit_BMI2) != 0;
        bool avx512f  = (ebx7 & bit_AVX512F) != 0;
        bool avx512bw = (ebx7 & bit_AVX512BW) != 0;
        bool avx512dq = (ebx7 & bit_AVX512DQ) != 0;

        // OS support for AVX/AVX512: check XGETBV (requires OSXSAVE)
        bool avx_os    = false;
        bool avx512_os = false;
        if (osxsave)
        {
            __asm__ __volatile__(
                "xgetbv"
                : "=a"(eax), "=d"(edx)
                : "c"(0)
                :
            );
            uint64_t xcr0 = ((uint64_t)edx << 32) | eax;

            // AVX requires XCR0[1:2] == 11b (XMM and YMM state)
            avx_os = (xcr0 & 0x6ULL) == 0x6ULL;
            // AVX512 requires XCR0[7,5,6] etc. common mask 0xE6 (bits 1,2,5,6,7)
            avx512_os = (xcr0 & 0xE6ULL) == 0xE6ULL;
        }

        cpu->march = "unknown";
        if (avx512f && avx512bw && avx512dq && avx512_os) cpu->march = "x86_64-v4";
        else if (avx2 && fma && bmi2 && avx_os) cpu->march = "x86_64-v3";
        else if (sse4_2 && popcnt && pclmul) cpu->march = "x86_64-v2";
        else if (sse2) cpu->march = "x86_64-v1";
    }
}

#else

inline static void ffCPUDetectByCpuid(FFCPUResult* cpu)
{
    // Unsupported platform
}

#endif
