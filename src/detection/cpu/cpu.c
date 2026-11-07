#include "cpu.h"

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu);

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu) {
    const char* error = ffDetectCPUImpl(options, cpu);
    if (error) {
        return error;
    }

    const char* removeStrings[] = {
        " CPU", " FPU", " APU", " Processor", " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core", " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core"
    };
    ffStrbufRemoveStrings(&cpu->name, ARRAY_SIZE(removeStrings), removeStrings);
    uint32_t radeonGraphics = ffStrbufFirstIndexS(&cpu->name, " w/ Radeon "); // w/ Radeon 780M Graphics
    if (radeonGraphics >= cpu->name.length) {
        radeonGraphics = ffStrbufFirstIndexS(&cpu->name, " with Radeon ");
    }
    if (radeonGraphics < cpu->name.length) {
        ffStrbufSubstrBefore(&cpu->name, radeonGraphics);
    }
    ffStrbufSubstrBeforeFirstC(&cpu->name, '@'); // Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&cpu->name, ' ');          // If we removed the @ in previous step there was most likely a space before it
    ffStrbufRemoveDupWhitespaces(&cpu->name);

#if __i386__ || __x86_64__
    ffCPUDetectX86Specific(cpu);
#endif

    return nullptr;
}

const char* ffCPUAppleCodeToName(uint32_t code) {
    // https://github.com/AsahiLinux/docs/blob/main/docs/hw/soc/soc-codenames.md
    switch (code) {
        case 8103:
            return "Apple M1";
        case 6000:
            return "Apple M1 Pro";
        case 6001:
            return "Apple M1 Max";
        case 6002:
            return "Apple M1 Ultra";
        case 8112:
            return "Apple M2";
        case 6020:
            return "Apple M2 Pro";
        case 6021:
            return "Apple M2 Max";
        case 6022:
            return "Apple M2 Ultra";
        case 8122:
            return "Apple M3";
        case 6030:
            return "Apple M3 Pro";
        case 6031:
        case 6034:
            return "Apple M3 Max";
        case 8132:
            return "Apple M4";
        case 6040:
            return "Apple M4 Pro";
        case 6041:
            return "Apple M4 Max";
        case 8140:
            return "Apple A18 Pro";
        case 8142:
            return "Apple M5";
        case 6050:
            return "Apple M5 Pro / Max";
        default:
            return nullptr;
    }
}

const char* ffCPUQualcommCodeToName(uint32_t code) {
    // https://github.com/AsahiLinux/docs/wiki/Codenames
    switch (code) {
        case 7180:
            return "Qualcomm Snapdragon 7c";
        case 7280:
            return "Qualcomm Snapdragon 7c+ Gen 3";
        case 8180:
            return "Qualcomm Snapdragon 8cx Gen 2 5G";
        case 8280:
            return "Qualcomm Snapdragon 8cx Gen 3";
        default:
            return nullptr;
    }
}

#if defined(__x86_64__) || defined(__i386__)

    #include <cpuid.h>

void ffCPUDetectByCpuid(FFCPUResult* cpu) {
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    if (__get_cpuid(0x16, &eax, &ebx, &ecx, &edx)) {
        // WARNING: CPUID may report frequencies of efficient cores
        // cpuid returns 0 MHz when hypervisor is enabled
        if (eax) {
            cpu->frequencyBase = eax;
        }
        if (ebx) {
            cpu->frequencyMax = ebx;
        }
    }

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        // Feature tests (leaf1.ecx, leaf7.ebx)
        bool sse2 = (ecx & bit_SSE2) != 0;
        bool sse4_2 = (ecx & bit_SSE4_2) != 0;
        bool pclmul = (ecx & bit_PCLMUL) != 0;
        bool popcnt = (ecx & bit_POPCNT) != 0;
        bool fma = (ecx & bit_FMA) != 0;
        bool osxsave = (ecx & bit_OSXSAVE) != 0;

        unsigned int eax7 = 0, ebx7 = 0, ecx7 = 0, edx7 = 0;
        __get_cpuid_count(7, 0, &eax7, &ebx7, &ecx7, &edx7);

        bool avx2 = (ebx7 & bit_AVX2) != 0;
        bool bmi2 = (ebx7 & bit_BMI2) != 0;
        bool avx512f = (ebx7 & bit_AVX512F) != 0;
        bool avx512bw = (ebx7 & bit_AVX512BW) != 0;
        bool avx512dq = (ebx7 & bit_AVX512DQ) != 0;

        // OS support for AVX/AVX512: check XGETBV (requires OSXSAVE)
        bool avx_os = false;
        bool avx512_os = false;
        if (osxsave) {
            __asm__ __volatile__(
                "xgetbv"
                : "=a"(eax), "=d"(edx)
                : "c"(0)
                :);
            uint64_t xcr0 = ((uint64_t) edx << 32) | eax;

            // AVX requires XCR0[1:2] == 11b (XMM and YMM state)
            avx_os = (xcr0 & 0x6ULL) == 0x6ULL;
            // AVX512 requires XCR0[7,5,6] etc. common mask 0xE6 (bits 1,2,5,6,7)
            avx512_os = (xcr0 & 0xE6ULL) == 0xE6ULL;
        }

        cpu->march = "unknown";
        if (avx512f && avx512bw && avx512dq && avx512_os) {
            cpu->march = "x86_64-v4";
        } else if (avx2 && fma && bmi2 && avx_os) {
            cpu->march = "x86_64-v3";
        } else if (sse4_2 && popcnt && pclmul) {
            cpu->march = "x86_64-v2";
        } else if (sse2) {
            cpu->march = "x86_64-v1";
        }
    }
}

#elif defined(__aarch64__)

// This is not accurate because a lot of flags are optional from old versions
// https://developer.arm.com/documentation/109697/2025_06/Feature-descriptions?lang=en
// https://en.wikipedia.org/wiki/AArch64#ARM-A_(application_architecture)
// Worth noting: Apple M1 is marked as ARMv8.5-A on Wikipedia, but it lacks BTI (mandatory in v8.5)

    #ifdef __linux__
        #include "common/io.h"
        #include <elf.h>
// #include <asm/hwcap.h>

void ffCPUDetectByCpuid(FFCPUResult* cpu) {
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/self/auxv", ARRAY_SIZE(buf), buf);

    if (nRead < (ssize_t) sizeof(Elf64_auxv_t)) {
        return;
    }

    uint64_t hwcap = 0, hwcap2 = 0;

    for (Elf64_auxv_t* auxv = (Elf64_auxv_t*) buf; (char*) auxv < buf + nRead; ++auxv) {
        if (auxv->a_type == AT_HWCAP) {
            hwcap = auxv->a_un.a_val;
        } else if (auxv->a_type == AT_HWCAP2) {
            hwcap2 = auxv->a_un.a_val;
        }
    }

    if (!hwcap) {
        return;
    }

    cpu->march = "unknown";

    // ARMv8-A
    bool has_fp = (hwcap & (1 << 0) /* HWCAP_FP */) != 0;
    bool has_asimd = (hwcap & (1 << 1) /* HWCAP_ASIMD */) != 0;

    // ARMv8.1-A
    bool has_atomics = (hwcap & (1 << 8) /* HWCAP_ATOMICS */) != 0;    // optional from v8.0
    bool has_crc32 = (hwcap & (1 << 7) /* HWCAP_CRC32 */) != 0;        // optional from v8.0
    bool has_asimdrdm = (hwcap & (1 << 12) /* HWCAP_ASIMDRDM */) != 0; // optional from v8.0

    // ARMv8.2-A
    bool has_fphp = (hwcap & (1 << 9) /* HWCAP_FPHP */) != 0;    // optional
    bool has_dcpop = (hwcap & (1 << 16) /* HWCAP_DCPOP */) != 0; // DC CVAP, optional from v8.1

    // ARMv8.3-A
    bool has_paca = (hwcap & (1 << 30) /* HWCAP_PACA */) != 0;   // optional from v8.2
    bool has_lrcpc = (hwcap & (1 << 15) /* HWCAP_LRCPC */) != 0; // optional from v8.2
    bool has_fcma = (hwcap & (1 << 14) /* HWCAP_FCMA */) != 0;   // optional from v8.2
    bool has_jscvt = (hwcap & (1 << 13) /* HWCAP_JSCVT */) != 0; // optional from v8.2

    // ARMv8.4-A
    bool has_dit = (hwcap & (1 << 24) /* HWCAP_DIT */) != 0;       // optional from v8.3
    bool has_flagm = (hwcap & (1 << 27) /* HWCAP_FLAGM */) != 0;   // optional from v8.1
    bool has_ilrcpc = (hwcap & (1 << 26) /* HWCAP_ILRCPC */) != 0; // optional from v8.2

    // ARMv8.5-A
    bool has_bti = (hwcap2 & (1 << 17) /* HWCAP2_BTI */) != 0;      // optional from v8.4
    bool has_sb = (hwcap & (1 << 29) /* HWCAP_SB */) != 0;          // optional from v8.0
    bool has_dcpodp = (hwcap2 & (1 << 0) /* HWCAP2_DCPODP */) != 0; // optional from v8.1
    bool has_flagm2 = (hwcap2 & (1 << 7) /* HWCAP2_FLAGM2 */) != 0; // optional from v8.4
    bool has_frint = (hwcap2 & (1 << 8) /* HWCAP2_FRINT */) != 0;   // optional from v8.4

    // ARMv9.0-A
    bool has_sve2 = (hwcap2 & (1 << 1) /* HWCAP2_SVE2 */) != 0;

    // ARMv9.1-A
    // ARMv8.6-A
    bool has_bf16 = (hwcap2 & (1 << 14) /* HWCAP2_BF16 */) != 0; // optional from v8.2
    bool has_i8mm = (hwcap2 & (1 << 13) /* HWCAP2_I8MM */) != 0; // optional from v8.1

    // ARMv8.7-A
    bool has_afp = (hwcap2 & (1 << 20) /* HWCAP2_AFP */) != 0; // optional from v8.6

    // ARMv9.2-A
    bool has_sme = (hwcap2 & (1 << 23) /* HWCAP2_SME */) != 0;

    // ARMv9.3-A
    bool has_sme2 = (hwcap2 & (1UL << 37) /* HWCAP2_SME2 */) != 0; // optional from v9.2

    // ARMv8.8-A
    bool has_mops = (hwcap2 & (1UL << 43) /* HWCAP2_MOPS */) != 0; // optional from v8.7

    // ARMv8.9-A
    bool has_cssc = (hwcap2 & (1UL << 34) /* HWCAP2_CSSC */) != 0; // optional from v8.7

    // ARMv9.4-A
    bool has_sme2p1 = (hwcap2 & (1UL << 38) /* HWCAP2_SME2P1 */) != 0; // optional from v9.2

    // ARMv9.5-A
    bool has_f8e4m3 = (hwcap2 & (1UL << 55) /* HWCAP2_F8E4M3 */) != 0; // optional from v9.2
    bool has_f8e5m2 = (hwcap2 & (1UL << 56) /* HWCAP2_F8E5M2 */) != 0; // optional from v9.2

    // ARMv9.6-A
    bool has_cmpbr = (hwcap & (1UL << 33) /* HWCAP_CMPBR */) != 0;   // optional from v9.5
    bool has_fprcvt = (hwcap & (1UL << 34) /* HWCAP_FPRCVT */) != 0; // optional from v9.5

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
// #include <arm/cpu_capabilities_public.h> // Not available in macOS 14-

void ffCPUDetectByCpuid(FFCPUResult* cpu) {
    uint64_t caps[2] = { 0 }; // 80-bit capability mask, split into two 64-bit values
    size_t size = sizeof(caps);

    if (sysctlbyname("hw.optional.arm.caps", caps, &size, nullptr, 0) != 0) {
        return;
    }

        // Helper macro to test bit in 80-bit capability mask
        #define FF_HAS_CAP(bit) \
            (((bit) < 64) ? ((caps[0] >> (bit)) & 1ULL) : ((caps[1] >> ((bit) - 64U)) & 1ULL))

    cpu->march = "unknown";

    // ARMv8-A
    bool has_fp = FF_HAS_CAP(50); /* CAP_BIT_AdvSIMD_HPFPCvt */ // Full FP16 support (implies FP/ASIMD)
    bool has_asimd = FF_HAS_CAP(49); /* CAP_BIT_AdvSIMD */      // Advanced SIMD (NEON)

    // ARMv8.1-A
    bool has_lse = FF_HAS_CAP(6); /* CAP_BIT_FEAT_LSE */      // Large System Extensions, optional in v8.0
    bool has_crc32 = FF_HAS_CAP(51); /* CAP_BIT_FEAT_CRC32 */ // CRC32 instructions, optional in v8.0
    bool has_rdm = FF_HAS_CAP(5); /* CAP_BIT_FEAT_RDM */      // AdvSIMD rounding double multiply accumulate, optional in v8.0

    // ARMv8.2-A
    bool has_fp16 = FF_HAS_CAP(34); /* CAP_BIT_FEAT_FP16 */ // Half-precision FP support, optional
    bool has_dpb = FF_HAS_CAP(22); /* CAP_BIT_FEAT_DPB */   // DC CVAP, optional from v8.1

    // ARMv8.3-A
    bool has_pauth = FF_HAS_CAP(19); /* CAP_BIT_FEAT_PAuth */ // Pointer Authentication (PAC), optional from v8.2
    bool has_lrcpc = FF_HAS_CAP(15); /* CAP_BIT_FEAT_LRCPC */ // LDAPR/LR with RCPC semantics, optional from v8.2
    bool has_fcma = FF_HAS_CAP(17); /* CAP_BIT_FEAT_FCMA */   // Complex number multiply-add, optional from v8.2
    bool has_jscvt = FF_HAS_CAP(18); /* CAP_BIT_FEAT_JSCVT */ // JavaScript-style conversion (FJCVTZS), optional from v8.2

    // ARMv8.4-A
    bool has_lse2 = FF_HAS_CAP(30); /* CAP_BIT_FEAT_LSE2 */     // Large System Extensions version 2, optional from v8.2
    bool has_dit = FF_HAS_CAP(33); /* CAP_BIT_FEAT_DIT */       // Data Independent Timing, optional from v8.3
    bool has_flagm = FF_HAS_CAP(0); /* CAP_BIT_FEAT_FlagM */    // Flag manipulation (FMOV/FCVT), optional from v8.1
    bool has_lrcpc2 = FF_HAS_CAP(16); /* CAP_BIT_FEAT_LRCPC2 */ // Enhanced RCPC (LDAPUR/LDAPST), optional from v8.2

    // ARMv8.5-A
    bool has_bti = FF_HAS_CAP(36); /* CAP_BIT_FEAT_BTI */         // Branch Target Identification, optional from v8.4
    bool has_sb = FF_HAS_CAP(13); /* CAP_BIT_FEAT_SB */           // Speculative Barrier, optional from v8.0
    bool has_dpb2 = FF_HAS_CAP(23); /* CAP_BIT_FEAT_DPB2 */       // DC CVADP (DPB2), optional from v8.1
    bool has_flagm2 = FF_HAS_CAP(1); /* CAP_BIT_FEAT_FlagM2 */    // Enhanced FlagM, optional from v8.4
    bool has_frintts = FF_HAS_CAP(14); /* CAP_BIT_FEAT_FRINTTS */ // Floating-point to integer instructions, optional from v8.4

    // ARMv9.0-A
    bool has_sve2 = false; // Not exposed and not supported by Apple M4

    // ARMv9.1-A
    // ARMv8.6-A
    bool has_bf16 = FF_HAS_CAP(24); /* CAP_BIT_FEAT_BF16 */ // Brain float16, optional from v8.2
    bool has_i8mm = FF_HAS_CAP(25); /* CAP_BIT_FEAT_I8MM */ // Int8 Matrix Multiply, optional from v8.1

    // ARMv8.7-A
    bool has_afp = FF_HAS_CAP(29); /* CAP_BIT_FEAT_AFP */ // Alternate FP16 (FEXPA), optional from v8.6

    // ARMv9.2-A
    bool has_sme = FF_HAS_CAP(40); /* CAP_BIT_FEAT_SME */ // Scalable Matrix Extension, optional from v9.2

    // ARMv9.3-A
    bool has_sme2 = FF_HAS_CAP(41); /* CAP_BIT_FEAT_SME2 */ // SME2, optional from v9.2

    // ARMv8.8-A
    bool has_hbc = FF_HAS_CAP(64); /* CAP_BIT_FEAT_HBC */ // Hinted conditional branches, optional from v8.7

    // ARMv8.9-A
    bool has_cssc = FF_HAS_CAP(67); /* CAP_BIT_FEAT_CSSC */ // Common Short String Compare, optional from v8.7

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
    #elif _WIN32
        #include "common/windows/registry.h"

// Windows exposes the AArch64 ID registers in the registry, as values named
// "CP xxxx" under HARDWARE\DESCRIPTION\System\CentralProcessor\0. Each value is
// the 64-bit content of one ID register; decode the same fields that the Linux
// HWCAP based detection below is built from.
//
// Where "xxxx" comes from
// -----------------------
// The four hex digits encode the AArch64 system register name S3_0_C0_CRm_op2
// (all ID registers live in CRn = C0 and use op0 = 3):
//
//     xxxx = 0x4000 + (CRm << 3) + op2
//
// e.g. ID_AA64PFR0_EL1 = S3_0_C0_C4_0 -> 0x4000 + (4 << 3) + 0 = 0x4020,
//      ID_AA64ISAR1_EL1 = S3_0_C0_C6_1 -> 0x4000 + (6 << 3) + 1 = 0x4031.
// The full list is in the ARM Architecture Reference Manual, chapter "AArch64
// System Register Encoding". Linux has the same encodings in sysreg.h as
// SYS_ID_AA64*_EL1; Windows just prints the encoding instead of the name.
//
// Where the offsets / minimums come from
// --------------------------------------
// Every field offset below is the LSB of that field in the ARM ARM description
// of the ID register (also SYS_ID_AA64*_EL1_*_SHIFT in Linux sysreg.h).
// Every minimum is the lowest field value that guarantees the corresponding
// instruction/feature (the field value tables in the ARM ARM, mirrored by the
// cpufeature.c checks that set the matching HWCAP/HWCAP2 bit):
//
//     value 0b0000  baseline value; usually "not implemented", but for fields
//                   that describe an always-present form it names that form
//                   (FP/AdvSIMD: implemented; SVEver: SVE; SMEver: SME)
//     value 0b0001  first extension level for most fields; SVEver 1 is SVE2
//     value 0b0010  second level; ATOMIC, LRCPC, DPB, FlagM, PFR1.SME, CSSC
//                   and SMFR0.SMEver use >= 2 for their newer form
//     value 0b1111  reserved "not implemented" marker; a field holding 0xF
//                   must never be treated as supporting the feature.
//
// armField() therefore requires both "field != 0xF" and
// "field >= minimum". minimum == 0 is used only where the mere presence of an
// implemented field is enough (PFR0.FP and PFR0.AdvSIMD for the ARMv8-A
// baseline). A register can be missing on some Windows builds/CPUs; a missing
// register makes all of its features false and thus lowers the reported
// version instead of reporting a wrong higher one.
//
// Not every field is a 4-bit value: ID_AA64FPFR0_EL1 (ARMv9.5 FP8) only
// defines individual feature bits, so those checks use armBit().
//
// To add a new instruction/feature in the future
// ----------------------------------------------
// 1. Find the ID register and field that advertises it in the ARM ARM (and in
//    the matching Linux HWCAP/HWCAP2 definition).
// 2. If that register is not read yet, add an FF_ARM_REG_* entry before
//    FF_ARM_REG_COUNT and its "CP xxxx" name in armRegisterNames[]; the xxxx is
//    computed with the formula above. Registers are read automatically because
//    armReadRegisters() loops over the whole table.
// 3. Add a bool using armField(regs, present, FF_ARM_REG_..., offset, minimum)
//    or armBit(regs, present, FF_ARM_REG_..., bit). Put the register/field name
//    and the Linux HWCAP bit it mirrors in a comment.
// 4. Insert the condition into the correct version's place in the if/else if
//    chain below, keeping the order identical to the Linux implementation so
//    both platforms report the same march. A newly required feature belongs to
//    the version that introduced it; a feature that only raises the baseline
//    is an extra `else if` above the older checks.
// 5. Rebuild and check with `fastfetch -s cpu --format json`; compare against
//    Linux on the same machine when possible.

typedef enum FFCPUArmRegister {
    FF_ARM_REG_PFR0,
    FF_ARM_REG_PFR1,
    FF_ARM_REG_ZFR0,
    FF_ARM_REG_SMFR0,
    FF_ARM_REG_FPFR0,
    FF_ARM_REG_ISAR0,
    FF_ARM_REG_ISAR1,
    FF_ARM_REG_ISAR2,
    FF_ARM_REG_ISAR3,
    FF_ARM_REG_MMFR1,
    FF_ARM_REG_COUNT,
} FFCPUArmRegister;

static const wchar_t* const armRegisterNames[FF_ARM_REG_COUNT] = {
    [FF_ARM_REG_PFR0] = L"CP 4020",  // ID_AA64PFR0_EL1
    [FF_ARM_REG_PFR1] = L"CP 4021",  // ID_AA64PFR1_EL1
    [FF_ARM_REG_ZFR0] = L"CP 4024",  // ID_AA64ZFR0_EL1
    [FF_ARM_REG_SMFR0] = L"CP 4025", // ID_AA64SMFR0_EL1
    [FF_ARM_REG_FPFR0] = L"CP 4027", // ID_AA64FPFR0_EL1
    [FF_ARM_REG_ISAR0] = L"CP 4030", // ID_AA64ISAR0_EL1
    [FF_ARM_REG_ISAR1] = L"CP 4031", // ID_AA64ISAR1_EL1
    [FF_ARM_REG_ISAR2] = L"CP 4032", // ID_AA64ISAR2_EL1
    [FF_ARM_REG_ISAR3] = L"CP 4033", // ID_AA64ISAR3_EL1
    [FF_ARM_REG_MMFR1] = L"CP 4039", // ID_AA64MMFR1_EL1
};

static bool armReadRegisters(uint64_t regs[FF_ARM_REG_COUNT], bool present[FF_ARM_REG_COUNT]) {
    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey, nullptr)) {
        return false;
    }

    bool anyData = false;
    for (uint32_t i = 0; i < FF_ARM_REG_COUNT; ++i) {
        uint64_t value = 0;
        if (ffRegReadUint64(hKey, armRegisterNames[i], &value, nullptr)) {
            regs[i] = value;
            present[i] = true;
            anyData = true;
        }
    }
    return anyData;
}

// Same rule as the Linux implementation: a 4-bit field is present when it is
// neither 0xF (not implemented) nor below the value required by the feature.
// `minimum` is the ARM ARM field value that first guarantees the feature; see
// the magic number explanation above.
static bool armField(
    const uint64_t regs[FF_ARM_REG_COUNT],
    const bool present[FF_ARM_REG_COUNT],
    FFCPUArmRegister reg,
    uint8_t offset,
    uint8_t minimum) {
    if (!present[reg]) {
        return false;
    }

    uint8_t value = (uint8_t) ((regs[reg] >> offset) & 0xF);
    return value != 0xF && value >= minimum;
}

// Single-bit feature fields (currently only ID_AA64FPFR0_EL1). Prefer
// armField() for the normal 4-bit ID fields.
static bool armBit(
    const uint64_t regs[FF_ARM_REG_COUNT],
    const bool present[FF_ARM_REG_COUNT],
    FFCPUArmRegister reg,
    uint8_t bit) {
    return present[reg] && ((regs[reg] >> bit) & 1) != 0;
}

void ffCPUDetectByCpuid(FFCPUResult* cpu) {
    uint64_t regs[FF_ARM_REG_COUNT] = {};
    bool present[FF_ARM_REG_COUNT] = {};

    if (!armReadRegisters(regs, present)) {
        return;
    }

    // ID_AA64ISAR1_EL1.APA [7:4] describes QARMA5 address authentication,
    // while API [11:8] describes the implementation-defined variant. They are
    // mutually exclusive but share the same encoding. Normalize API into the
    // APA slot so both implementations are detected (Apple Silicon reports
    // APA = 0 and API = 2).
    uint64_t isar1 = regs[FF_ARM_REG_ISAR1];
    uint8_t apa = (uint8_t) ((isar1 >> 4) & 0xF);
    uint8_t api = (uint8_t) ((isar1 >> 8) & 0xF);
    if (apa == 0 && api != 0) {
        isar1 = (isar1 & ~(UINT64_C(0xF) << 4)) | ((uint64_t) api << 4);
        regs[FF_ARM_REG_ISAR1] = isar1;
    }

    cpu->march = "unknown";

    // ARMv8-A
    bool has_fp = armField(regs, present, FF_ARM_REG_PFR0, 16, 0);    // PFR0.FP[19:16] implemented -> HWCAP_FP
    bool has_asimd = armField(regs, present, FF_ARM_REG_PFR0, 20, 0); // PFR0.AdvSIMD[23:20] implemented -> HWCAP_ASIMD

    // ARMv8.1-A
    bool has_atomics = armField(regs, present, FF_ARM_REG_ISAR0, 20, 2);  // ISAR0.ATOMIC[23:20] >= 2 -> HWCAP_ATOMICS
    bool has_crc32 = armField(regs, present, FF_ARM_REG_ISAR0, 16, 1);    // ISAR0.CRC32[19:16] >= 1 -> HWCAP_CRC32
    bool has_asimdrdm = armField(regs, present, FF_ARM_REG_ISAR0, 28, 1); // ISAR0.RDM[31:28] >= 1 -> HWCAP_ASIMDRDM

    // ARMv8.2-A
    bool has_fphp = armField(regs, present, FF_ARM_REG_PFR0, 16, 1);  // PFR0.FP[19:16] >= 1 -> HWCAP_FPHP
    bool has_dcpop = armField(regs, present, FF_ARM_REG_ISAR1, 0, 1); // ISAR1.DPB[3:0] >= 1 -> HWCAP_DCPOP

    // ARMv8.3-A
    bool has_paca = armField(regs, present, FF_ARM_REG_ISAR1, 4, 1)    // ISAR1.APA[7:4] >= 1 -> HWCAP_PACA
        || armField(regs, present, FF_ARM_REG_ISAR2, 12, 1);           // ISAR2.APA3[15:12] >= 1 (newer encoding)
    bool has_lrcpc = armField(regs, present, FF_ARM_REG_ISAR1, 20, 1); // ISAR1.LRCPC[23:20] >= 1 -> HWCAP_LRCPC
    bool has_fcma = armField(regs, present, FF_ARM_REG_ISAR1, 16, 1);  // ISAR1.FCMA[19:16] >= 1 -> HWCAP_FCMA
    bool has_jscvt = armField(regs, present, FF_ARM_REG_ISAR1, 12, 1); // ISAR1.JSCVT[15:12] >= 1 -> HWCAP_JSCVT

    // ARMv8.4-A
    bool has_dit = armField(regs, present, FF_ARM_REG_PFR0, 48, 1);     // PFR0.DIT[51:48] >= 1 -> HWCAP_DIT
    bool has_flagm = armField(regs, present, FF_ARM_REG_ISAR0, 52, 1);  // ISAR0.FlagM[55:52] >= 1 -> HWCAP_FLAGM
    bool has_ilrcpc = armField(regs, present, FF_ARM_REG_ISAR1, 20, 2); // ISAR1.LRCPC[23:20] >= 2 -> HWCAP_ILRCPC

    // ARMv8.5-A
    bool has_bti = armField(regs, present, FF_ARM_REG_PFR1, 0, 1);      // PFR1.BT[3:0] >= 1 -> HWCAP2_BTI
    bool has_sb = armField(regs, present, FF_ARM_REG_ISAR1, 36, 1);     // ISAR1.SB[39:36] >= 1 -> HWCAP_SB
    bool has_dcpodp = armField(regs, present, FF_ARM_REG_ISAR1, 0, 2);  // ISAR1.DPB[3:0] >= 2 -> HWCAP2_DCPODP
    bool has_flagm2 = armField(regs, present, FF_ARM_REG_ISAR0, 52, 2); // ISAR0.FlagM[55:52] >= 2 -> HWCAP2_FLAGM2
    bool has_frint = armField(regs, present, FF_ARM_REG_ISAR1, 32, 1);  // ISAR1.FRINTTS[35:32] >= 1 -> HWCAP2_FRINT

    // ARMv9.0-A
    bool has_sve2 = armField(regs, present, FF_ARM_REG_ZFR0, 0, 1); // ZFR0.SVEver[3:0] >= 1 (1 = SVE2) -> HWCAP2_SVE2

    // ARMv9.1-A
    // ARMv8.6-A
    // BF16/I8MM are shared by both version sequences, so the same booleans are
    // tested in both branches of the chain below.
    bool has_bf16 = armField(regs, present, FF_ARM_REG_ISAR1, 44, 1); // ISAR1.BF16[47:44] >= 1 -> HWCAP2_BF16
    bool has_i8mm = armField(regs, present, FF_ARM_REG_ISAR1, 52, 1); // ISAR1.I8MM[55:52] >= 1 -> HWCAP2_I8MM

    // ARMv8.7-A
    bool has_afp = armField(regs, present, FF_ARM_REG_MMFR1, 44, 1); // MMFR1.AFP[47:44] >= 1 -> HWCAP2_AFP

    // ARMv9.2-A
    bool has_sme = armField(regs, present, FF_ARM_REG_PFR1, 24, 1); // PFR1.SME[27:24] >= 1 -> HWCAP2_SME

    // ARMv9.3-A
    bool has_sme2 = armField(regs, present, FF_ARM_REG_PFR1, 24, 2); // PFR1.SME[27:24] >= 2 -> HWCAP2_SME2

    // ARMv8.8-A
    bool has_mops = armField(regs, present, FF_ARM_REG_ISAR2, 16, 1); // ISAR2.MOPS[19:16] >= 1 -> HWCAP2_MOPS

    // ARMv8.9-A
    bool has_cssc = armField(regs, present, FF_ARM_REG_ISAR2, 52, 1); // ISAR2.CSSC[55:52] >= 1 -> HWCAP2_CSSC

    // ARMv9.4-A
    bool has_sme2p1 = armField(regs, present, FF_ARM_REG_SMFR0, 56, 2); // SMFR0.SMEver[59:56] >= 2 -> HWCAP2_SME2P1

    // ARMv9.5-A
    bool has_f8e4m3 = armBit(regs, present, FF_ARM_REG_FPFR0, 1); // FPFR0.F8E4M3[1] -> HWCAP2_F8E4M3
    bool has_f8e5m2 = armBit(regs, present, FF_ARM_REG_FPFR0, 0); // FPFR0.F8E5M2[0] -> HWCAP2_F8E5M2

    // ARMv9.6-A
    bool has_cmpbr = armField(regs, present, FF_ARM_REG_ISAR2, 52, 2);  // ISAR2.CSSC[55:52] >= 2 -> HWCAP_CMPBR
    bool has_fprcvt = armField(regs, present, FF_ARM_REG_ISAR3, 28, 1); // ISAR3.FPRCVT[31:28] >= 1 -> HWCAP_FPRCVT

    // The version chain (both the ARMv9/ARMv8 split and the order of the
    // checks) must stay identical to the Linux implementation above, otherwise
    // the same CPU reports a different march depending on the OS.
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
    #else
void ffCPUDetectByCpuid([[maybe_unused]] FFCPUResult* cpu) {
    // Unsupported system
}
    #endif

#else

void ffCPUDetectByCpuid([[maybe_unused]] FFCPUResult* cpu) {
    // Unsupported architecture
}

#endif
