#include "cpu.h"

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu);

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu)
{
    const char* error = ffDetectCPUImpl(options, cpu);
    if (error) return error;

    const char* removeStrings[] = {
        " CPU", " FPU", " APU", " Processor",
        " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core",
        " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core"
    };
    ffStrbufRemoveStrings(&cpu->name, ARRAY_SIZE(removeStrings), removeStrings);
    uint32_t radeonGraphics = ffStrbufFirstIndexS(&cpu->name, " w/ Radeon "); // w/ Radeon 780M Graphics
    if (radeonGraphics >= cpu->name.length)
        radeonGraphics = ffStrbufFirstIndexS(&cpu->name, " with Radeon ");
    if (radeonGraphics < cpu->name.length)
        ffStrbufSubstrBefore(&cpu->name, radeonGraphics);
    ffStrbufSubstrBeforeFirstC(&cpu->name, '@'); //Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&cpu->name, ' '); //If we removed the @ in previous step there was most likely a space before it
    ffStrbufRemoveDupWhitespaces(&cpu->name);
    return NULL;
}

const char* ffCPUAppleCodeToName(uint32_t code)
{
    // https://github.com/AsahiLinux/docs/wiki/Codenames
    switch (code)
    {
        case 8103: return "Apple M1";
        case 6000: return "Apple M1 Pro";
        case 6001: return "Apple M1 Max";
        case 6002: return "Apple M1 Ultra";
        case 8112: return "Apple M2";
        case 6020: return "Apple M2 Pro";
        case 6021: return "Apple M2 Max";
        case 6022: return "Apple M2 Ultra";
        case 8122: return "Apple M3";
        case 6030: return "Apple M3 Pro";
        case 6031:
        case 6034: return "Apple M3 Max";
        case 8132: return "Apple M4";
        case 6040: return "Apple M4 Pro";
        case 6041: return "Apple M4 Max";
        default: return NULL;
    }
}

const char* ffCPUQualcommCodeToName(uint32_t code)
{
    // https://github.com/AsahiLinux/docs/wiki/Codenames
    switch (code)
    {
        case 7180: return "Qualcomm Snapdragon 7c";
        case 7280: return "Qualcomm Snapdragon 7c+ Gen 3";
        case 8180: return "Qualcomm Snapdragon 8cx Gen 2 5G";
        case 8280: return "Qualcomm Snapdragon 8cx Gen 3";
        default: return NULL;
    }
}

#if defined(__x86_64__) || defined(__i386__)

#include <cpuid.h>

void ffCPUDetectByCpuid(FFCPUResult* cpu)
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

#elif defined(__aarch64__)

// This is not accurate because a lot of flags are optional from old versions
// https://developer.arm.com/documentation/109697/2025_06/Feature-descriptions?lang=en
// https://en.wikipedia.org/wiki/AArch64#ARM-A_(application_architecture)
// Worth noting: Apple M1 is marked as ARMv8.5-A on Wikipedia, but it lacks BTI (mandatory in v8.5)

#ifdef __linux__
#include "common/io/io.h"
#include <elf.h>
#include <asm/hwcap.h>

#ifndef HWCAP2_SME
#define HWCAP2_SME         (1UL << 23)
#endif
#ifndef HWCAP2_SME2
#define HWCAP2_SME2        (1UL << 37)
#endif
#ifndef HWCAP2_CSSC
#define HWCAP2_CSSC        (1UL << 34)
#endif
#ifndef HWCAP2_SME2P1
#define HWCAP2_SME2P1      (1UL << 38)
#endif
#ifndef HWCAP2_MOPS
#define HWCAP2_MOPS        (1UL << 43)
#endif
#ifndef HWCAP2_F8E4M3
#define HWCAP2_F8E4M3      (1UL << 55)
#endif
#ifndef HWCAP2_F8E5M2
#define HWCAP2_F8E5M2      (1UL << 56)
#endif
#ifndef HWCAP_CMPBR
#define HWCAP_CMPBR        (1UL << 33)
#endif
#ifndef HWCAP_FPRCVT
#define HWCAP_FPRCVT       (1UL << 34)
#endif

void ffCPUDetectByCpuid(FFCPUResult* cpu)
{
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/self/auxv", ARRAY_SIZE(buf), buf);

    if (nRead < (ssize_t) sizeof(Elf64_auxv_t)) return;

    uint64_t hwcap = 0, hwcap2 = 0;

    for (Elf64_auxv_t* auxv = (Elf64_auxv_t*)buf; (char*)auxv < buf + nRead; ++auxv)
    {
        if (auxv->a_type == AT_HWCAP)
        {
            hwcap = auxv->a_un.a_val;
        }
        else if (auxv->a_type == AT_HWCAP2)
        {
            hwcap2 = auxv->a_un.a_val;
        }
    }

    if (!hwcap) return;

    cpu->march = "unknown";

    // ARMv8-A
    bool has_fp = (hwcap & HWCAP_FP) != 0;
    bool has_asimd = (hwcap & HWCAP_ASIMD) != 0;

    // ARMv8.1-A
    bool has_atomics = (hwcap & HWCAP_ATOMICS) != 0; // optional from v8.0
    bool has_crc32 = (hwcap & HWCAP_CRC32) != 0; // optional from v8.0
    bool has_asimdrdm = (hwcap & HWCAP_ASIMDRDM) != 0; // optional from v8.0

    // ARMv8.2-A
    bool has_fphp = (hwcap & HWCAP_FPHP) != 0; // optional
    bool has_dcpop = (hwcap & HWCAP_DCPOP) != 0; // DC CVAP, optional from v8.1

    // ARMv8.3-A
    bool has_paca = (hwcap & HWCAP_PACA) != 0; // optional from v8.2
    bool has_lrcpc = (hwcap & HWCAP_LRCPC) != 0; // optional from v8.2
    bool has_fcma = (hwcap & HWCAP_FCMA) != 0; // optional from v8.2
    bool has_jscvt = (hwcap & HWCAP_JSCVT) != 0; // optional from v8.2

    // ARMv8.4-A
    bool has_dit = (hwcap & HWCAP_DIT) != 0; // optional from v8.3
    bool has_flagm = (hwcap & HWCAP_FLAGM) != 0; // optional from v8.1
    bool has_ilrcpc = (hwcap & HWCAP_ILRCPC) != 0; // optional from v8.2

    // ARMv8.5-A
    bool has_bti = (hwcap2 & HWCAP2_BTI) != 0; // optional from v8.4
    bool has_sb = (hwcap & HWCAP_SB) != 0; // optional from v8.0
    bool has_dcpodp = (hwcap2 & HWCAP2_DCPODP) != 0; // optional from v8.1
    bool has_flagm2 = (hwcap2 & HWCAP2_FLAGM2) != 0; // optional from v8.4
    bool has_frint = (hwcap2 & HWCAP2_FRINT) != 0; // optional from v8.4

    // ARMv9.0-A
    bool has_sve2 = (hwcap2 & HWCAP2_SVE2) != 0;

    // ARMv9.1-A
    // ARMv8.6-A
    bool has_bf16 = (hwcap2 & HWCAP2_BF16) != 0; // optional from v8.2
    bool has_i8mm = (hwcap2 & HWCAP2_I8MM) != 0; // optional from v8.1

    // ARMv8.7-A
    bool has_afp = (hwcap2 & HWCAP2_AFP) != 0; // optional from v8.6

    // ARMv9.2-A
    bool has_sme = (hwcap2 & HWCAP2_SME) != 0;

    // ARMv9.3-A
    bool has_sme2 = (hwcap2 & HWCAP2_SME2) != 0; // optional from v9.2

    // ARMv8.8-A
    bool has_mops = (hwcap2 & HWCAP2_MOPS) != 0; // optional from v8.7

    // ARMv8.9-A
    bool has_cssc = (hwcap2 & HWCAP2_CSSC) != 0; // optional from v8.7

    // ARMv9.4-A
    bool has_sme2p1 = (hwcap2 & HWCAP2_SME2P1) != 0; // optional from v9.2

    // ARMv9.5-A
    bool has_f8e4m3 = (hwcap2 & HWCAP2_F8E4M3) != 0; // optional from v9.2
    bool has_f8e5m2 = (hwcap2 & HWCAP2_F8E5M2) != 0; // optional from v9.2

    // ARMv9.6-A
    bool has_cmpbr = (hwcap & HWCAP_CMPBR) != 0; // optional from v9.5
    bool has_fprcvt = (hwcap & HWCAP_FPRCVT) != 0; // optional from v9.5

    if (has_sve2 || has_sme) {
        // ARMv9
        if (has_cmpbr && has_fprcvt) {
            cpu->march = "ARMv9.6-A";
        } else if (has_f8e5m2 && has_f8e4m3) {
            cpu->march = "ARMv9.5-A";
        } else if (has_sme2p1) {
            cpu->march = "ARMv9.4-A";
        } else if (has_sme2) {
            cpu->march = "ARMv9.3-A";
        } else if (has_sme) {
            cpu->march = "ARMv9.2-A";
        } else if (has_i8mm && has_bf16) {
            cpu->march = "ARMv9.1-A";
        } else {
            cpu->march = "ARMv9.0-A";
        }
    } else {
        // ARMv8
        if (has_cssc) {
            cpu->march = "ARMv8.9-A";
        } else if (has_mops) {
            cpu->march = "ARMv8.8-A";
        } else if (has_afp) {
            cpu->march = "ARMv8.7-A";
        } else if (has_i8mm && has_bf16) {
            cpu->march = "ARMv8.6-A";
        } else if (has_bti && has_sb && has_dcpodp && has_flagm2 && has_frint) {
            cpu->march = "ARMv8.5-A";
        } else if (has_dit && has_flagm && has_ilrcpc) {
            cpu->march = "ARMv8.4-A";
        } else if (has_paca && has_lrcpc && has_fcma && has_jscvt) {
            cpu->march = "ARMv8.3-A";
        } else if (has_fphp && has_dcpop) {
            cpu->march = "ARMv8.2-A";
        } else if (has_atomics && has_crc32 && has_asimdrdm) {
            cpu->march = "ARMv8.1-A";
        } else if (has_asimd && has_fp) {
            cpu->march = "ARMv8-A";
        }
    }
}
#elif __APPLE__
#include <sys/sysctl.h>
#include <arm/cpu_capabilities_public.h>

#ifndef CAP_BIT_AdvSIMD
#define CAP_BIT_AdvSIMD             49
#endif
#ifndef CAP_BIT_AdvSIMD_HPFPCvt
#define CAP_BIT_AdvSIMD_HPFPCvt     50
#endif
#ifndef CAP_BIT_FEAT_CRC32
#define CAP_BIT_FEAT_CRC32          51
#endif
#ifndef CAP_BIT_FEAT_HBC
#define CAP_BIT_FEAT_HBC            64
#endif
#ifndef CAP_BIT_FEAT_CSSC
#define CAP_BIT_FEAT_CSSC           67
#endif

void ffCPUDetectByCpuid(FFCPUResult* cpu)
{
    uint64_t caps[2] = {0}; // 80-bit capability mask, split into two 64-bit values
    size_t size = sizeof(caps);

    if (sysctlbyname("hw.optional.arm.caps", caps, &size, NULL, 0) != 0) return;

    // Helper macro to test bit in 80-bit capability mask
    #define FF_HAS_CAP(bit) \
        (((bit) < 64) ? ((caps[0] >> (bit)) & 1ULL) : ((caps[1] >> ((bit) - 64U)) & 1ULL))

    cpu->march = "unknown";

    // ARMv8-A
    bool has_fp        = FF_HAS_CAP(CAP_BIT_AdvSIMD_HPFPCvt); // Full FP16 support (implies FP/ASIMD)
    bool has_asimd     = FF_HAS_CAP(CAP_BIT_AdvSIMD);         // Advanced SIMD (NEON)

    // ARMv8.1-A
    bool has_lse       = FF_HAS_CAP(CAP_BIT_FEAT_LSE);        // Large System Extensions, optional in v8.0
    bool has_crc32     = FF_HAS_CAP(CAP_BIT_FEAT_CRC32);      // CRC32 instructions, optional in v8.0
    bool has_rdm       = FF_HAS_CAP(CAP_BIT_FEAT_RDM);        // AdvSIMD rounding double multiply accumulate, optional in v8.0

    // ARMv8.2-A
    bool has_fp16      = FF_HAS_CAP(CAP_BIT_FEAT_FP16);       // Half-precision FP support, optional
    bool has_dpb       = FF_HAS_CAP(CAP_BIT_FEAT_DPB);        // DC CVAP, optional from v8.1

    // ARMv8.3-A
    bool has_pauth     = FF_HAS_CAP(CAP_BIT_FEAT_PAuth);      // Pointer Authentication (PAC), optional from v8.2
    bool has_lrcpc     = FF_HAS_CAP(CAP_BIT_FEAT_LRCPC);      // LDAPR/LR with RCPC semantics, optional from v8.2
    bool has_fcma      = FF_HAS_CAP(CAP_BIT_FEAT_FCMA);       // Complex number multiply-add, optional from v8.2
    bool has_jscvt     = FF_HAS_CAP(CAP_BIT_FEAT_JSCVT);      // JavaScript-style conversion (FJCVTZS), optional from v8.2

    // ARMv8.4-A
    bool has_lse2      = FF_HAS_CAP(CAP_BIT_FEAT_LSE2);       // Large System Extensions version 2, optional from v8.2
    bool has_dit       = FF_HAS_CAP(CAP_BIT_FEAT_DIT);        // Data Independent Timing, optional from v8.3
    bool has_flagm     = FF_HAS_CAP(CAP_BIT_FEAT_FlagM);      // Flag manipulation (FMOV/FCVT), optional from v8.1
    bool has_lrcpc2    = FF_HAS_CAP(CAP_BIT_FEAT_LRCPC2);     // Enhanced RCPC (LDAPUR/LDAPST), optional from v8.2

    // ARMv8.5-A
    bool has_bti       = FF_HAS_CAP(CAP_BIT_FEAT_BTI);        // Branch Target Identification, optional from v8.4
    bool has_sb        = FF_HAS_CAP(CAP_BIT_FEAT_SB);         // Speculative Barrier, optional from v8.0
    bool has_dpb2      = FF_HAS_CAP(CAP_BIT_FEAT_DPB2);       // DC CVADP (DPB2), optional from v8.1
    bool has_flagm2    = FF_HAS_CAP(CAP_BIT_FEAT_FlagM2);     // Enhanced FlagM, optional from v8.4
    bool has_frintts   = FF_HAS_CAP(CAP_BIT_FEAT_FRINTTS);    // Floating-point to integer instructions, optional from v8.4

    // ARMv9.0-A
    bool has_sve2      = false;                               // Not exposed and not supported by Apple M4

    // ARMv9.1-A
    // ARMv8.6-A
    bool has_bf16      = FF_HAS_CAP(CAP_BIT_FEAT_BF16);       // Brain float16, optional from v8.2
    bool has_i8mm      = FF_HAS_CAP(CAP_BIT_FEAT_I8MM);       // Int8 Matrix Multiply, optional from v8.1

    // ARMv8.7-A
    bool has_afp       = FF_HAS_CAP(CAP_BIT_FEAT_AFP);        // Alternate FP16 (FEXPA), optional from v8.6

    // ARMv9.2-A
    bool has_sme       = FF_HAS_CAP(CAP_BIT_FEAT_SME);        // Scalable Matrix Extension, optional from v9.2

    // ARMv9.3-A
    bool has_sme2      = FF_HAS_CAP(CAP_BIT_FEAT_SME2);       // SME2, optional from v9.2

    // ARMv8.8-A
    bool has_hbc       = FF_HAS_CAP(CAP_BIT_FEAT_HBC);        // Hinted conditional branches, optional from v8.7

    // ARMv8.9-A
    bool has_cssc      = FF_HAS_CAP(CAP_BIT_FEAT_CSSC);       // Common Short String Compare, optional from v8.7

    // ARMv9.4-A+ are not exposed yet

    if (has_sve2 || has_sme) {
        // ARMv9 family
        if (has_sme2) {
            cpu->march = "ARMv9.3-A";
        } else if (has_sme) {
            cpu->march = "ARMv9.2-A";
        } else if (has_i8mm && has_bf16) {
            cpu->march = "ARMv9.1-A";
        } else {
            cpu->march = "ARMv9.0-A";
        }
    } else {
        // ARMv8 family
        if (has_cssc) {
            cpu->march = "ARMv8.9-A";
        } else if (has_hbc) {
            cpu->march = "ARMv8.8-A";
        } else if (has_afp) {
            cpu->march = "ARMv8.7-A";
        } else if (has_i8mm && has_bf16) {
            cpu->march = "ARMv8.6-A";
        } else if (has_bti && has_sb && has_dpb2 && has_flagm2 && has_frintts) {
            cpu->march = "ARMv8.5-A";
        } else if (has_lse2 && has_dit && has_flagm && has_lrcpc2) {
            cpu->march = "ARMv8.4-A";
        } else if (has_pauth && has_lrcpc && has_fcma && has_jscvt) {
            cpu->march = "ARMv8.3-A";
        } else if (has_fp16 && has_dpb) {
            cpu->march = "ARMv8.2-A";
        } else if (has_lse && has_crc32 && has_rdm) {
            cpu->march = "ARMv8.1-A";
        } else if (has_asimd && has_fp) {
            cpu->march = "ARMv8-A";
        }
    }

    #undef HAS_CAP
}
#else
#endif // __linux__

#else

void ffCPUDetectByCpuid(FF_MAYBE_UNUSED FFCPUResult* cpu)
{
    // Unsupported platform
}

#endif
