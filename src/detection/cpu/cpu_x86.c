#include "cpu.h"

#if __x86_64__ || __i386__
    #include <cpuid.h>
    #include "common/strutil.h"

    #define UNKN_STR "Unknown"

typedef struct match_entry_t {
    int32_t family, model, stepping, ext_family, ext_model; // -1 means wildcard
    int32_t ncores, l2cache, l3cache;
    struct {
        const char* pattern;
        int32_t score;
    } brand;
    const char* name;
    const char* technology;
} FFCPUX86MatchEntry;

///////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

// From libcpuid: https://github.com/anrieff/libcpuid
// LICENSE: BSD-2-Clause

/****************************************** Intel ****************************************** */

// https://github.com/anrieff/libcpuid/blob/2e4456ae0165db3155da2e8fba92afd5c090ca1b/libcpuid/recog_intel.c
/*
 * Copyright 2008  Veselin Georgiev,
 * anrieffNOSPAM @ mgail_DOT.com (convert to gmail)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Useful links:
 * - List of Intel CPU microarchitectures: https://en.wikipedia.org/wiki/List_of_Intel_CPU_microarchitectures
 * - List of Intel processors: https://en.wikipedia.org/wiki/List_of_Intel_processors
 * - List of Intel Pentium processors: https://en.wikipedia.org/wiki/List_of_Intel_Pentium_processors
 * - List of Intel Celeron processors: https://en.wikipedia.org/wiki/List_of_Intel_Celeron_processors
 * - List of Intel Core processors: https://en.wikipedia.org/wiki/List_of_Intel_Core_processors
 * - List of Intel Xeon processors: https://en.wikipedia.org/wiki/List_of_Intel_Xeon_processors
*/
const struct match_entry_t cpudb_intel[] = {
//     F   M   S  EF    EM #cores L2$    L3$ Pattern                 Codename                   Technology
	{ -1, -1, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "Unknown Intel CPU",       UNKN_STR },

	/* i486 */
	{  4, -1, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "Unknown i486",            UNKN_STR },
	{  4,  0, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 DX-25/33",           "1 µm"   },
	{  4,  1, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 DX-50",              "0.8 µm" },
	{  4,  2, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 SX",                 UNKN_STR },
	{  4,  3, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 DX2",                UNKN_STR },
	{  4,  4, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 SL",                 "0.8 µm" },
	{  4,  5, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 SX2",                UNKN_STR },
	{  4,  7, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 DX2 WriteBack",      UNKN_STR },
	{  4,  8, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 DX4",                "0.6 µm" },
	{  4,  9, -1, -1, -1,   1,    -1,    -1, { "",              0 }, "i486 DX4 WriteBack",      "0.6 µm" },

	/* P6 CPUs */
	{  5,  0, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium A-Step",    UNKN_STR  },
	{  5,  1, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium 1",         "0.8 µm"  },
	{  5,  2, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium 1",         "0.35 µm" },
	{  5,  3, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium OverDrive", UNKN_STR  },
	{  5,  4, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium 1",         "0.35 µm" },
	{  5,  7, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium 1",         "0.35 µm" },
	{  5,  8, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium MMX",       "0.25 µm" },

	/* P6 CPUs */
	{  6,  0, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium Pro",                UNKN_STR  },
	{  6,  1, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium Pro",                UNKN_STR  },
	{  6,  3, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium II (Klamath)",       "0.18 µm" },
	{  6,  5, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium II (Deschutes)",     "0.18 µm" },
	{  6,  5, -1, -1, -1,   1,    -1,    -1, { "Pentium(R) M",  4 }, "Mobile Pentium II (Tonga)",  "0.18 µm" },
	{  6,  6, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium II (Dixon)",         "0.25 µm" },
	{  6,  3, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-II Xeon (Klamath)",        "0.35 µm" },
	{  6,  5, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-II Xeon (Drake)",          "0.25 µm" },
	{  6,  6, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-II Xeon (Dixon)",          "0.25 µm" },
	{  6,  5, -1, -1, -1,   1,    -1,    -1, { "Celeron(R)",    2 }, "P-II Celeron (Covington)",   "0.25 µm" },
	{  6,  6, -1, -1, -1,   1,    -1,    -1, { "Celeron(R)",    2 }, "P-II Celeron (Mendocino)",   "0.25 µm" },
	{  6,  7, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium III (Katmai)",       "0.25 µm" },
	{  6,  8, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium III (Coppermine)",   "0.18 µm" },
	{  6, 10, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium III (Coppermine)",   "0.18 µm" },
	{  6, 11, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",    2 }, "Pentium III (Tualatin)",     "0.13 µm" },
	{  6, 11, -1, -1, -1,   1,   512,    -1, { "Pentium(R)",    2 }, "Pentium III (Tualatin)",     "0.13 µm" },
	{  6,  7, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-III Xeon (Tanner)",        "0.25 µm" },
	{  6,  8, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-III Xeon (Cascades)",      "0.18 µm" },
	{  6, 10, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-III Xeon (Cascades)",      "0.18 µm" },
	{  6, 11, -1, -1, -1,   1,    -1,    -1, { "Xeon(TM)",      2 }, "P-III Xeon (Tualatin)",      "0.13 µm" },
	{  6,  7, -1, -1, -1,   1,   128,    -1, { "Celeron(R)",    2 }, "P-III Celeron (Katmai)",     "0.25 µm" },
	{  6,  8, -1, -1, -1,   1,   128,    -1, { "Celeron(R)",    2 }, "P-III Celeron (Coppermine)", "0.18 µm" },
	{  6, 10, -1, -1, -1,   1,   128,    -1, { "Celeron(R)",    2 }, "P-III Celeron (Coppermine)", "0.18 µm" },
	{  6, 11, -1, -1, -1,   1,   256,    -1, { "Celeron(R)",    2 }, "P-III Celeron (Tualatin)",   "0.13 µm" },

	/* NetBurst CPUs */
	/* Willamette (2000, 180 nm): */
	{ 15,  0, -1, 15, -1,   1,    -1,    -1, { "Pentium(R)",        2 }, "Pentium 4 (Willamette)",   "0.18 µm" },
	{ 15,  1, -1, 15, -1,   1,    -1,    -1, { "Pentium(R)",        2 }, "Pentium 4 (Willamette)",   "0.18 µm" },
	{ 15,  0, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Mobile P-4 (Willamette)",  "0.18 µm" },
	{ 15,  1, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Mobile P-4 (Willamette)",  "0.18 µm" },
	{ 15,  1, -1, 15, -1,   1,    -1,    -1, { "Celeron(R)",        2 }, "P-4 Celeron (Willamette)", "0.18 µm" },
	{ 15,  0, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Foster)",            "0.18 µm" },
	{ 15,  1, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Foster)",            "0.18 µm" },
	/* Northwood / Mobile Pentium 4 / Banias (2002, 130 nm): */
	{ 15,  2, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4",      2 }, "Pentium 4 (Northwood)",   "0.13 µm" },
	{ 15,  2, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Mobile P-4 (Northwood)",  "0.13 µm" },
	{ 15,  2, -1, 15, -1,   1,    -1,    -1, { "Celeron(R)",        2 }, "P-4 Celeron (Northwood)", "0.13 µm" },
	{  6,  9, -1, -1, -1,   1,    -1,    -1, { "Pentium(R)",        2 }, "Pentium M (Banias)",      "0.13 µm" },
	{  6,  9, -1, -1, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Pentium M (Banias)",      "0.13 µm" },
	{  6,  9, -1, -1, -1,   1,    -1,    -1, { "Celeron(R)",        2 }, "Celeron M (Banias)",      "0.13 µm" },
	{  6,  9, -1, -1, -1,   1,    -1,    -1, { "Celeron(R) M",      4 }, "Celeron M (Banias)",      "0.13 µm" },
	{  6,  9, -1, -1, -1,   1,     0,    -1, { "Celeron(R)",        2 }, "Celeron M (Shelton)",     "0.13 µm" },
	{ 15,  2, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Prestonia)",        "0.13 µm" },
	{ 15,  2, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM) MP",       4 }, "Xeon (Gallatin)",         "0.13 µm" },
	/* Prescott / Dothan (2004, 90 nm): */
	{ 15,  3, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4",      4 }, "Pentium 4 (Prescott)",     "90 nm" },
	{ 15,  4, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4",      4 }, "Pentium 4 (Prescott)",     "90 nm" },
	{ 15,  3, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Mobile P-4 (Prescott)",    "90 nm" },
	{ 15,  4, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Mobile P-4 (Prescott)",    "90 nm" },
	{ 15,  3, -1, 15, -1,   1,    -1,    -1, { "Celeron(R)",        2 }, "P-4 Celeron D (Prescott)", "90 nm" },
	{ 15,  4, -1, 15, -1,   1,    -1,    -1, { "Celeron(R)",        2 }, "P-4 Celeron D (Prescott)", "90 nm" },
	{ 15,  4, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) D",      4 }, "Pentium D (SmithField)",   "90 nm" },
	{  6, 13, -1, -1, -1,   1,    -1,    -1, { "Pentium(R) M",      4 }, "Pentium M (Dothan)",       "90 nm" },
	{  6, 13, -1, -1, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",  6 }, "Pentium M (Dothan)",       "90 nm" },
	{  6, 13, -1, -1, -1,   1,    -1,    -1, { "Celeron(R)",        2 }, "Celeron M (Dothan)",       "90 nm" },
	{  6, 13, -1, -1, -1,   1,    -1,    -1, { "Celeron(R) M",      4 }, "Celeron M (Dothan)",       "90 nm" },
	{ 15,  3, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Nocona)",            "90 nm" },
	{ 15,  4, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Nocona)",            "90 nm" },
	{ 15,  4,  3, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Irwindale)",         "90 nm" },
	{ 15,  4, 10, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Irwindale)",         "90 nm" },
	{ 15,  4,  1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Cranford)",          "90 nm" },
	{ 15,  4, -1, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Potomac)",           "90 nm" },
	{ 15,  4,  8, 15, -1,   1,    -1,    -1, { "Xeon(TM)",          2 }, "Xeon (Paxville)",          "90 nm" },
	/* Cedar Mill / Yonah / Presler (2006, 65 nm): */
	{ 15,  6, -1, 15, -1,   1,    -1,    -1, { "Pentium(R)",                 2 }, "Pentium 4 (Cedar Mill)",            "65 nm" },
	{ 15,  6, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) 4 - M",           6 }, "Mobile P-4 (Cedar Mill)",           "65 nm" },
	{ 15,  6, -1, 15, -1,   1,    -1,    -1, { "Celeron(R) D",               4 }, "P-4 Celeron D (Cedar Mill)",        "65 nm" },
	{  6, 14, -1, 14, -1,   1,    -1,    -1, { "Core(TM) [UT]1#[05]0",       6 }, "Core Solo (Yonah)",                 "65 nm" },
	{  6, 14, -1, 14, -1,   1,    -1,    -1, { "Core(TM) 1#[05]0",           4 }, "Core Solo (Yonah)",                 "65 nm" },
	{  6, 14, -1, 14, -1,   2,    -1,    -1, { "Core(TM) Duo [UTL]2#[05]#",  6 }, "Core Duo (Yonah)",                  "65 nm" },
	{  6, 14, -1, 14, -1,   2,    -1,    -1, { "Core(TM) Duo 2#[05]#",       4 }, "Core Duo (Yonah)",                  "65 nm" },
	{  6, 14, -1, 14, -1,  -1,    -1,    -1, { "Celeron(R) 215",             6 }, "Celeron (Yonah-512)",               "65 nm" },
	{  6, 14, -1, 14, -1,  -1,    -1,    -1, { "Celeron(R) M",               4 }, "Celeron (Yonah-1024)",              "65 nm" },
	{ 15,  6, -1, 15, -1,   1,    -1,    -1, { "Pentium(R) D",               4 }, "Pentium D (Presler)",               "65 nm" },
	{ 15,  6,  2, 15, -1,   1,    -1,    -1, { "Pentium(R)",                 2 }, "Pentium Extreme Edition (Presler)", "65 nm" },
	{ 15,  6,  4, 15, -1,   1,    -1,    -1, { "Xeon(TM)",                   2 }, "Xeon (Dempsey)",                    "65 nm" },
	{ 15,  6,  6, 15, -1,   1,    -1,    -1, { "Xeon(TM)",                   2 }, "Xeon (Tulsa)",                      "65 nm" },
	{ 15,  6,  8, 15, -1,   1,    -1,    -1, { "Xeon(TM)",                   2 }, "Xeon (Tulsa)",                      "65 nm" },

	/* Core CPUs (2006, 65 nm): https://en.wikipedia.org/wiki/Intel_Core_(microarchitecture)*/
	{  6, 15, -1, -1, -1,   2,  2048,    -1, { "Core(TM)2 Duo E6###",    8 }, "Core 2 Duo (Conroe-2M)",        "65 nm" },
	{  6, 15, -1, -1, -1,   2,  4096,    -1, { "Core(TM)2 Duo E6###",    8 }, "Core 2 Duo (Conroe)",           "65 nm" },
	{  6, 15, -1, -1, -1,   2,  4096,    -1, { "Core(TM)2 6###",         4 }, "Core 2 Duo (Conroe)",           "65 nm" },
	{  6, 15, -1, -1, -1,   2,  4096,    -1, { "Core(TM)2 X6###",        6 }, "Core 2 Extreme (Conroe XE)",    "65 nm" },
	{  6, 15, -1, -1, -1,   4,  4096,    -1, { "Core(TM)2 Quad Q6###",   8 }, "Core 2 Quad (Kentsfield)",      "65 nm" },
	{  6, 15, -1, -1, -1,   2,  2048,    -1, { "Core(TM)2 Duo E4###",    8 }, "Core 2 Duo (Allendale)",        "65 nm" },
	{  6, 15, -1, -1, 15,   2,  2048,    -1, { "Core(TM)2 U7###",        6 }, "Core 2 Duo (Merom-2M)",         "65 nm" },
	{  6, 15, -1, -1, 15,   2,  2048,    -1, { "Core(TM)2 T[57]###",     6 }, "Core 2 Duo (Merom-2M)",         "65 nm" },
	{  6, 15, -1, -1, 15,   2,  4096,    -1, { "Core(TM)2 T7###",        6 }, "Core 2 Duo (Merom)",            "65 nm" },
	{  6, 15, -1, -1, 15,   2,  4096,    -1, { "Core(TM)2 S[LP]7###",    6 }, "Core 2 Duo (Merom)",            "65 nm" },
	{  6, 15, -1, -1, 15,   2,  4096,    -1, { "Core(TM)2 L7###",        6 }, "Core 2 Duo (Merom)",            "65 nm" },
	{  6, 15, -1, -1, 15,   2,    -1,    -1, { "Pentium(R) Dual E2###",  8 }, "Pentium Dual-Core (Allendale)", "65 nm" },
	{  6, 15, -1, -1, 15,   2,    -1,    -1, { "Celeron(R) E1###",       6 }, "Celeron (Allendale)",           "65 nm" },
	{  6,  6, -1, -1, 22,   1,    -1,    -1, { "Celeron(R) [24]##",      4 }, "Celeron (Conroe-L)",            "65 nm" },
	{  6, 14, -1, -1, 14,   1,    -1,    -1, { "Xeon(R) 51##",           4 }, "Xeon LV (Woodcrest)",           "65 nm" },
	{  6, 15, -1, -1, 15,   2,    -1,    -1, { "Xeon(R) 51##",           4 }, "Xeon (Woodcrest)",              "65 nm" },
	{  6, 15, -1, -1, 15,   2,    -1,    -1, { "Xeon(R) 30##",           4 }, "Xeon (Conroe)",                 "65 nm" },
	{  6, 15, -1, -1, 15,   4,    -1,    -1, { "Xeon(R) X32##",          6 }, "Xeon (Kentsfield)",             "65 nm" },
	{  6, 15, -1, -1, 15,   4,    -1,    -1, { "Xeon(R) [EXL]53##",      6 }, "Xeon (Clovertown)",             "65 nm" },

	/* Penryn CPUs (2007, 45 nm): https://en.wikipedia.org/wiki/Penryn_(microarchitecture) */
	{  6,  7, -1, -1, 23,   2,  1024,    -1, { "Celeron(R) E3###",            6 }, "Celeron (Wolfdale-3M)",        "45 nm" },
	{  6,  7, -1, -1, 23,   2,  1024,    -1, { "Pentium(R) E2###",            6 }, "Celeron (Wolfdale-3M)",        "45 nm" },
	{  6,  7, -1, -1, 23,   2,  2048,    -1, { "Pentium(R) E[56]###",         6 }, "Pentium (Wolfdale-3M)",        "45 nm" },
	{  6,  7, -1, -1, 23,   2,  3072,    -1, { "Core(TM)2 Duo E7###",         8 }, "Core 2 Duo (Wolfdale-3M)",     "45 nm" },
	{  6,  7, -1, -1, 23,   2,  6144,    -1, { "Core(TM)2 Duo E8###",         8 }, "Core 2 Duo (Wolfdale)",        "45 nm" },
	{  6,  7, -1, -1, 23,   2,  1024,    -1, { "Pentium(R) Dual-Core T4###",  8 }, "Pentium Dual-Core (Penryn-L)", "45 nm" },
	{  6,  7, -1, -1, 23,   1,  1024,    -1, { "Celeron(R) [79]##",           4 }, "Celeron (Penryn-L)",           "45 nm" },
	{  6,  7, -1, -1, 23,   2,  3072,    -1, { "Core(TM)2 Duo SU[78]###",     8 }, "Core 2 Duo (Penryn-3M)",       "45 nm" },
	{  6,  7, -1, -1, 23,   2,  3072,    -1, { "Core(TM)2 Duo P[78]###",      8 }, "Core 2 Duo (Penryn-3M)",       "45 nm" },
	{  6,  7, -1, -1, 23,   2,  2048,    -1, { "Core(TM)2 Duo T6###",         8 }, "Core 2 Duo (Penryn-3M)",       "45 nm" },
	{  6,  7, -1, -1, 23,   2,  3072,    -1, { "Core(TM)2 Duo T8###",         8 }, "Core 2 Duo (Penryn-3M)",       "45 nm" },
	{  6,  7, -1, -1, 23,   2,  6144,    -1, { "Core(TM)2 Duo S[LP]9###",     8 }, "Core 2 Duo (Penryn)",          "45 nm" },
	{  6,  7, -1, -1, 23,   2,  6144,    -1, { "Core(TM)2 Duo [PT]9###",      8 }, "Core 2 Duo (Penryn)",          "45 nm" },
	{  6,  7, -1, -1, 23,   2,  6144,    -1, { "Core(TM)2 Duo E8###",         8 }, "Core 2 Duo (Penryn)",          "45 nm" },
	{  6,  7, -1, -1, 23,   4,  2048,    -1, { "Core(TM)2 Quad Q8###",        8 }, "Core 2 Quad (Yorkfield-6M)",   "45 nm" }, /* 2×2 MB L2$ */
	{  6,  7, -1, -1, 23,   4,  3072,    -1, { "Core(TM)2 Quad Q9#0#",        8 }, "Core 2 Quad (Yorkfield-6M)",   "45 nm" }, /* 2×3 MB L2$ */
	{  6,  7, -1, -1, 23,   4,  6144,    -1, { "Core(TM)2 Quad Q9#5#",        8 }, "Core 2 Quad (Yorkfield)",      "45 nm" }, /* 2×6 MB L2$ */
	{  6,  7, -1, -1, 23,   2,    -1,    -1, { "Xeon(R) [EL]31##",            6 }, "Xeon (Wolfdale)",              "45 nm" },
	{  6,  7, -1, -1, 23,   2,    -1,    -1, { "Xeon(R) [EXL]52##",           6 }, "Xeon (Wolfdale DP)",           "45 nm" },
	{  6,  7, -1, -1, 23,   4,    -1,    -1, { "Xeon(R) [EXL]54##",           6 }, "Xeon (Harpertown)",            "45 nm" },
	{  6,  7, -1, -1, 23,   4,    -1,    -1, { "Xeon(R) [XL]33##",            6 }, "Xeon (Yorkfield)" ,            "45 nm" },
	{  6, 13, -1, -1, 29,  -1,    -1,    -1, { "Xeon(R) [EXL]74##",           6 }, "Xeon (Dunnington)",            "45 nm" },

	/* Nehalem CPUs (2008, 1st Core i gen, 45 nm): https://en.wikipedia.org/wiki/Nehalem_(microarchitecture) */
	{  6, 10, -1, -1, 26,  -1,    -1,    -1, { "Xeon(R) [WELX]5###",        6 }, "Xeon (Gainestown)",            "45 nm" },
	{  6, 10, -1, -1, 26,  -1,    -1,    -1, { "Xeon(R) [WELX]3###",        6 }, "Xeon (Bloomfield)",            "45 nm" },
	{  6, 10, -1, -1, 26,  -1,    -1,    -1, { "Core(TM) i7 9#5",           8 }, "Core i7 Extreme (Bloomfield)", "45 nm" },
	{  6, 10, -1, -1, 26,  -1,    -1,    -1, { "Core(TM) i7 9#0",           8 }, "Core i7 (Bloomfield)",         "45 nm" },
	{  6, 14, -1, -1, 30,  -1,    -1,    -1, { "Core(TM) i7 8##",           8 }, "Core i7 (Lynnfield)",          "45 nm" },
	{  6, 14, -1, -1, 30,  -1,    -1,    -1, { "Core(TM) i5 7##",           8 }, "Core i5 (Lynnfield)",          "45 nm" },
	{  6, 14, -1, -1, 30,  -1,    -1,    -1, { "Core(TM) i7 [QX] [789]##", 10 }, "Core i7 (Clarksfield)",        "45 nm" },
	{  6, 14, -1, -1, 30,  -1,    -1,    -1, { "Core(TM) [QX] [789]##",     8 }, "Core i7 (Clarksfield)",        "45 nm" },

	/* Bonnell CPUs (2008, Atom, 45 nm): https://en.wikipedia.org/wiki/Bonnell_(microarchitecture) */
	{  6,  6, -1, -1, 38,  -1,    -1,    -1, { "Atom(TM) E6##T",       8 }, "Atom (Tunnel Creek)", "45 nm" },
	{  6,  6, -1, -1, 38,  -1,    -1,    -1, { "Atom(TM) E6##",        6 }, "Atom (Tunnel Creek)", "45 nm" },
	{  6,  6, -1, -1, 38,  -1,    -1,    -1, { "Atom(TM) E6##C",       8 }, "Atom (Stellarton)",   "45 nm" },
	{  6, 12, -1, -1, 28,  -1,    -1,    -1, { "Atom(TM) Z5##",        6 }, "Atom (Silverthorne)", "45 nm" },
	{  6, 12, -1, -1, 28,  -1,    -1,    -1, { "Atom(TM) N2##",        6 }, "Atom (Diamondville)", "45 nm" },
	{  6, 12, -1, -1, 28,  -1,    -1,    -1, { "Atom(TM) [23]##",      6 }, "Atom (Diamondville)", "45 nm" },
	{  6,  6, -1, -1, 38,  -1,    -1,    -1, { "Atom(TM) Z6##",        6 }, "Atom (Lincroft)",     "45 nm" },
	{  6, 12, -1, -1, 28,  -1,    -1,    -1, { "Atom(TM) Z6##",        6 }, "Atom (Lincroft)",     "45 nm" },
	{  6, 12, -1, -1, 28,  -1,    -1,    -1, { "Atom(TM) [ND][45]##",  6 }, "Atom (Pineview)",     "45 nm" },

	/* Westmere CPUs (2010, 1st Core i gen, 32 nm): https://en.wikipedia.org/wiki/Westmere_(microarchitecture) */
	{  6, 14, -1, -1, 46,  -1,    -1,    -1, { "Xeon(R) [EXL]75##",        6 }, "Xeon 7000 (Beckton)",         "32 nm" },
	{  6, 14, -1, -1, 46,  -1,    -1,    -1, { "Xeon(R) E65##",            6 }, "Xeon 6000 (Beckton)",         "32 nm" },
	{  6, 14, -1, -1, 46,  -1,    -1,    -1, { "Xeon(R) [XELW]5[56]##",    6 }, "Xeon 5000 (Beckton)",         "32 nm" },
	{  6, 14, -1, -1, 46,  -1,    -1,    -1, { "Xeon(R) [XLW]3[456]###",   6 }, "Xeon 3000 (Beckton)",         "32 nm" },
	{  6, 15, -1, -1, 47,  -1,    -1,    -1, { "Xeon(R) E7-#8##",          6 }, "Xeon E7 (Westmere-EX)",       "32 nm" },
	{  6, 12, -1, -1, 44,  -1,    -1,    -1, { "Xeon(R) [XEL]5###",        6 }, "Xeon (Westmere-EP)",          "32 nm" },
	{  6, 12, -1, -1, 44,  -1,    -1,    -1, { "Xeon(R) W3###",            6 }, "Xeon (Gulftown)",             "32 nm" },
	{  6, 12, -1, -1, 44,  -1,    -1,    -1, { "Core(TM) i7 X 9##",       10 }, "Core i7 Extreme (Gulftown)",  "32 nm" },
	{  6, 12, -1, -1, 44,  -1,    -1,    -1, { "Core(TM) i7 9##",          8 }, "Core i7 (Gulftown)",          "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Xeon(R) L3###",            6 }, "Xeon (Clarkdale)",            "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Core(TM) i5 6##",          8 }, "Core i5 (Clarkdale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Core(TM) i3 5##",          8 }, "Core i3 (Clarkdale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Pentium(R) G6###",         6 }, "Pentium (Clarkdale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Celeron(R) G1###",         6 }, "Celeron (Clarkdale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Core(TM) i7 M 6##",        8 }, "Core i7 (Arrandale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Core(TM) i5 M [45]##",     8 }, "Core i5 (Arrandale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Core(TM) i3 M 3##",        8 }, "Core i3 (Arrandale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Pentium(R) P6###",         6 }, "Pentium (Arrandale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Pentium(R) 5U###",         6 }, "Pentium (Arrandale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Celeron(R) P4###",         6 }, "Celeron (Arrandale)",         "32 nm" },
	{  6,  5, -1, -1, 37,  -1,    -1,    -1, { "Celeron(R) U3###",         6 }, "Celeron (Arrandale)",         "32 nm" },

	/* Saltwell CPUs (2011, Atom, 32 nm): https://en.wikipedia.org/wiki/Bonnell_(microarchitecture)#Third_generation_cores */
	{  6, 12, -1, -1, -1,  -1,    -1,    -1, { "Atom(TM) [ND]2###",    6 }, "Atom (Cedarview)",    "32 nm" },
	{  6,  6, -1, -1, 54,  -1,    -1,    -1, { "Atom(TM) [ND]2###",    6 }, "Atom (Cedarview)",    "32 nm" },
	{  6,  7, -1, -1, 39,  -1,    -1,    -1, { "Atom(TM) Z2###",       6 }, "Atom (Penwell)",      "32 nm" },

	/* Sandy Bridge CPUs (2011, 2nd Core i gen, 32 nm): https://en.wikipedia.org/wiki/Sandy_Bridge */
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Xeon(R) E5####[LW]",    8 }, "Xeon E5 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Xeon(R) E5####",        6 }, "Xeon E5 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Xeon(R) E3####[CL]",    8 }, "Xeon E3 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Xeon(R) E3####",        6 }, "Xeon E3 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Core(TM) i7-2###",      8 }, "Core i7 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Core(TM) i5-2###",      8 }, "Core i5 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Core(TM) i3-2###",      8 }, "Core i3 (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Pentium(R) G[68]##",    6 }, "Pentium (Sandy Bridge)",           "32 nm" },
	{  6, 10, -1, -1, 42,  -1,    -1,    -1, { "Celeron(R) G[45]##",    6 }, "Celeron (Sandy Bridge)",           "32 nm" },
	{  6, 13, -1, -1, 45,  -1,    -1,    -1, { "Core(TM) i7-3###[KX]", 10 }, "Core i7 Extreme (Sandy Bridge-E)", "32 nm" },
	{  6, 13, -1, -1, 45,  -1,    -1,    -1, { "Xeon(R) E5-####",       4 }, "Xeon E5 (Sandy Bridge-E)",         "32 nm" },
	{  6, 13, -1, -1, 45,  -1,    -1,    -1, { "Xeon(R) E3-####",       4 }, "Xeon E3 (Sandy Bridge-E)",         "32 nm" },

	/* Ivy Bridge CPUs (2012, 3rd Core i gen, 22 nm): https://en.wikipedia.org/wiki/Ivy_Bridge_(microarchitecture) */
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Xeon(R) E7-####L v2",     8 }, "Xeon E7 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Xeon(R) E7-#### v2",      6 }, "Xeon E7 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Xeon(R) E5-####[LW] v2",  8 }, "Xeon E5 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Xeon(R) E5-#### v2",      6 }, "Xeon E5 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Xeon(R) E3-####[CL] v2",  8 }, "Xeon E3 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Xeon(R) E3-#### v2",      6 }, "Xeon E3 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Core(TM) i7-3###",        8 }, "Core i7 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Core(TM) i5-3###",        8 }, "Core i5 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Core(TM) i3-3###",        8 }, "Core i3 (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Pentium(R) G2###",        6 }, "Pentium (Ivy Bridge)",           "22 nm" },
	{  6, 10, -1, -1, 58,  -1,    -1,    -1, { "Celeron(R) G1###",        6 }, "Celeron (Ivy Bridge)",           "22 nm" },
	{  6, 14, -1, -1, 62,  -1,    -1,    -1, { "Xeon(R) E7-#### v2",      6 }, "Xeon E7 (Ivy Bridge-E)",         "22 nm" },
	{  6, 14, -1, -1, 62,  -1,    -1,    -1, { "Xeon(R) E5-#### v2",      6 }, "Xeon E5 (Ivy Bridge-E)",         "22 nm" },
	{  6, 14, -1, -1, 62,  -1,    -1,    -1, { "Xeon(R) E3-#### v2",      6 }, "Xeon E3 (Ivy Bridge-E)",         "22 nm" },
	{  6, 14, -1, -1, 62,  -1,    -1,    -1, { "Core(TM) i7-4###X",      10 }, "Core i7 Extreme (Ivy Bridge-E)", "22 nm" },
	{  6, 14, -1, -1, 62,  -1,    -1,    -1, { "Core(TM) i7-4###K",       8 }, "Core i7 (Ivy Bridge-E)",         "22 nm" },

	/* Silvermont CPUs (2013, Atom, 22 nm): https://en.wikipedia.org/wiki/Silvermont */
	{  6,  7, -1, -1,  55,  -1,    -1,    -1, { "Pentium(R) J2###",  6 }, "Pentium (Bay Trail-D)", "22 nm" },
	{  6,  7, -1, -1,  55,  -1,    -1,    -1, { "Celeron(R) J1###",  6 }, "Celeron (Bay Trail-D)", "22 nm" },
	{  6,  7, -1, -1,  55,  -1,    -1,    -1, { "Pentium(R) N3###",  6 }, "Pentium (Bay Trail-M)", "22 nm" },
	{  6,  7, -1, -1,  55,  -1,    -1,    -1, { "Celeron(R) N2###",  6 }, "Celeron (Bay Trail-M)", "22 nm" },
	{  6,  7, -1, -1,  55,  -1,    -1,    -1, { "Atom(TM) Z3###",    6 }, "Atom (Bay Trail-T)",    "22 nm" },
	{  6,  7, -1, -1,  55,  -1,    -1,    -1, { "Atom(TM) E3###",    6 }, "Atom (Bay Trail-I)",    "22 nm" },
	{  6, 13, -1, -1,  77,  -1,    -1,    -1, { "Atom(TM) C2##0",    8 }, "Atom (Avoton)",         "22 nm" },
	{  6, 13, -1, -1,  77,  -1,    -1,    -1, { "Atom(TM) C2##[68]", 8 }, "Atom (Rangeley)",       "22 nm" },

	/* Haswell CPUs (2013, 4th Core i gen, 22 nm): https://en.wikipedia.org/wiki/Haswell_(microarchitecture) */
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Xeon(R) E7-####L v3",       8 }, "Xeon E7 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Xeon(R) E7-#### v3",        6 }, "Xeon E7 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Xeon(R) E5-####[ABLW] v3",  8 }, "Xeon E5 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Xeon(R) E5-#### v3",        6 }, "Xeon E5 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Xeon(R) E3-####L v3",       8 }, "Xeon E3 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Xeon(R) E3-#### v3",        6 }, "Xeon E3 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Core(TM) i7-4###",          8 }, "Core i7 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Core(TM) i5-4###",          8 }, "Core i5 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Core(TM) i3-4###",          8 }, "Core i3 (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Pentium(R) G3###",          6 }, "Pentium (Haswell)",         "22 nm" },
	{  6, 12, -1, -1, 60,  -1,    -1,    -1, { "Celeron(R) G1###",          6 }, "Celeron (Haswell)",         "22 nm" },
	{  6, 15, -1, -1, 63,  -1,    -1,    -1, { "Core(TM) i7-5###[KX]",      8 }, "Core i7 Extreme (Haswell)", "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i7-4###",          8 }, "Core i7 (Haswell)",         "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i5-4###",          8 }, "Core i5 (Haswell)",         "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i3-4###",          8 }, "Core i3 (Haswell)",         "22 nm" },
	{  6,  6, -1, -1, 70,  -1,    -1,    -1, { "Core(TM) i7-4###R",        10 }, "Core i7 (Haswell-H)",       "22 nm" }, /* GT3e */
	{  6,  6, -1, -1, 70,  -1,    -1,    -1, { "Core(TM) i5-4###R",        10 }, "Core i5 (Haswell-H)",       "22 nm" }, /* GT3e */
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i7-4###U",        10 }, "Core i7 (Haswell-ULT)",     "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i5-4###U",        10 }, "Core i5 (Haswell-ULT)",     "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i3-4###U",        10 }, "Core i3 (Haswell-ULT)",     "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i7-4###Y",        10 }, "Core i7 (Haswell-ULX)",     "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i5-4###Y",        10 }, "Core i5 (Haswell-ULX)",     "22 nm" },
	{  6,  5, -1, -1, 69,  -1,    -1,    -1, { "Core(TM) i3-4###Y",        10 }, "Core i3 (Haswell-ULX)",     "22 nm" },

	/* Broadwell CPUs (2014, 5th Core i gen, 14 nm): https://en.wikipedia.org/wiki/Broadwell_(microarchitecture) */
	{  6,  6, -1, -1, 86,  -1,    -1,    -1, { "Xeon(R) D-15##",              6 }, "Xeon D (Broadwell)",    "14 nm" },
	{  6,  6, -1, -1, 86,  -1,    -1,    -1, { "Pentium(R) D15##",            6 }, "Pentium D (Broadwell)", "14 nm" },
	{  6,  7, -1, -1, 71,   4,    -1,    -1, { "Core(TM) i7-5###[CR]",       10 }, "Core i7 (Broadwell-H)", "14 nm" },
	{  6,  7, -1, -1, 71,   4,    -1,    -1, { "Core(TM) i5-5###[CR]",       10 }, "Core i5 (Broadwell-H)", "14 nm" },
	{  6, 13, -1, -1, 61,   4,    -1,    -1, { "Core(TM) i7-5###HQ",         12 }, "Core i7 (Broadwell-U)", "14 nm" },
	{  6, 13, -1, -1, 61,   2,    -1,    -1, { "Core(TM) i7-5###U",          10 }, "Core i7 (Broadwell-U)", "14 nm" },
	{  6, 13, -1, -1, 61,   2,    -1,    -1, { "Core(TM) i5-5###[HU]",       10 }, "Core i5 (Broadwell-U)", "14 nm" },
	{  6, 13, -1, -1, 61,   2,    -1,    -1, { "Core(TM) i3-5###U",          10 }, "Core i3 (Broadwell-U)", "14 nm" },
	{  6, 13, -1, -1, 61,   2,    -1,    -1, { "Pentium(R) 3###U",            6 }, "Pentium (Broadwell-U)", "14 nm" },
	{  6, 13, -1, -1, 61,   2,    -1,    -1, { "Celeron(R) 3###U",            6 }, "Celeron (Broadwell-U)", "14 nm" },
	{  6, 13, -1, -1, 61,   2,    -1,    -1, { "5Y##",                        4 }, "Core M (Broadwell-Y)",  "14 nm" },
	{  6, 15, -1, -1, 79,  -1,    -1,    -1, { "Xeon(R) E7-#### v4",          6 }, "Xeon E7 (Broadwell)",   "14 nm" },
	{  6, 15, -1, -1, 79,  -1,    -1,    -1, { "Xeon(R) E5-####[ACLPRW] v4",  8 }, "Xeon E5 (Broadwell)",   "14 nm" },
	{  6, 15, -1, -1, 79,  -1,    -1,    -1, { "Xeon(R) E5-#### v4",          6 }, "Xeon E5 (Broadwell)",   "14 nm" },
	{  6, 15, -1, -1, 79,  -1,    -1,    -1, { "Xeon(R) E3-####L v4",         8 }, "Xeon E3 (Broadwell)",   "14 nm" },
	{  6, 15, -1, -1, 79,  -1,    -1,    -1, { "Xeon(R) E3-#### v4",          6 }, "Xeon E3 (Broadwell)",   "14 nm" },
	{  6, 15, -1, -1, 79,   4,    -1,    -1, { "Core(TM) i7-6###[KX]",       10 }, "Core i7 (Broadwell-E)", "14 nm" },

	/* Airmont CPUs (2014, Atom, 14 nm): https://en.wikipedia.org/wiki/Silvermont#List_of_Airmont_processors */
	{  6, 12, -1, -1,  76, -1,    -1,    -1, { "Pentium(R) [JN]3###",  6 }, "Pentium (Braswell)",     "14 nm" },
	{  6, 12, -1, -1,  76, -1,    -1,    -1, { "Celeron(R) [JN]3###",  6 }, "Celeron (Braswell)",     "14 nm" },
	{  6, 12, -1, -1,  76,  4,    -1,    -1, { "Atom(TM) x7-Z8###",    8 }, "Atom x7 (Cherry Trail)", "14 nm" },
	{  6, 12, -1, -1,  76,  4,    -1,    -1, { "Atom(TM) x5-Z8###",    8 }, "Atom x5 (Cherry Trail)", "14 nm" },
	{  6,  5, -1, -1, 117, -1,    -1,    -1, { "Spreadtrum",           2 }, "Spreadtrum (Airmont)",   "14 nm" }, /* Spreadtrum SC9853I-IA */

	/* Skylake (client) CPUs (2015, 6th Core i gen, 14 nm): https://en.wikichip.org/wiki/intel/microarchitectures/skylake_(client) */
	{  6, 14, -1, -1, 94,   4,    -1,    -1, { "Core(TM) i7-6###",         8 }, "Core i7 (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 94,   4,    -1,    -1, { "Core(TM) i5-6###",         8 }, "Core i5 (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 94,   2,    -1,    -1, { "Core(TM) i3-6###",         8 }, "Core i3 (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 94,   2,    -1,    -1, { "Pentium(R) G4###",         6 }, "Pentium (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 94,   2,    -1,    -1, { "Celeron(R) G3###",         6 }, "Celeron (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 78,   2,    -1,    -1, { "Core(TM) m7-6Y##",         8 }, "Core m7 (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 78,   2,    -1,    -1, { "Core(TM) m5-6Y##",         8 }, "Core m5 (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 78,   2,    -1,    -1, { "Core(TM) m3-6Y##",         8 }, "Core m3 (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 78,   2,    -1,    -1, { "Pentium(R) 4###[UY]",      6 }, "Pentium (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 78,   2,    -1,    -1, { "Celeron(R) 3###U",         6 }, "Celeron (Skylake)",   "14 nm" },
	{  6, 14, -1, -1, 78,   2,    -1,    -1, { "Celeron(R) G3###E",        8 }, "Celeron (Skylake)",   "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Core(TM) i9-7###X",       10 }, "Core i9 (Skylake-X)", "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Core(TM) i7-7###X",       10 }, "Core i7 (Skylake-X)", "14 nm" }, /* Core i7 7800X + 7820X */
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Core(TM) i9-9###X",       10 }, "Core i9 (Skylake-X)", "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Core(TM) i7-9###X",       10 }, "Core i7 (Skylake-X)", "14 nm" }, /* Core i7 9800X */
	{  6, 14, -1, -1, 94,  -1,    -1,    -1, { "Xeon(R) W-#1##X",          8 }, "Xeon (Skylake-X)",    "14 nm" },
	/* Skylake (server) CPUs (2017, 1st Xeon Scalable gen, 14 nm): https://en.wikichip.org/wiki/intel/microarchitectures/skylake_(server) */
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) D-#1##",          6 }, "Xeon D (Skylake-D)",         "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) E3-####[ML] v5",  8 }, "Xeon E3 (Skylake-S)",        "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) E3-#### v5",      6 }, "Xeon E3 (Skylake-S)",        "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) W-#1##",          6 }, "Xeon W (Skylake-W)",         "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Platinum #1##",   6 }, "Xeon Platinum (Skylake-SP)", "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Gold #1##",       6 }, "Xeon Gold (Skylake-SP)",     "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Silver #1##",     6 }, "Xeon Silver (Skylake-SP)",   "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Bronze #1##",     6 }, "Xeon Bronze (Skylake-SP)",   "14 nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Montage(R) Jintide(R)",   4 }, "Jintide (Skylake-SP)",       "14 nm" }, /* Montage(R) Jintide(R) C2460 */
	/* Kaby Lake CPUs (2016, 7th Core i gen, 14+ nm): https://en.wikipedia.org/wiki/Kaby_Lake */
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i7-7###",          8 }, "Core i7 (Kaby Lake)",         "14+ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-7###",          8 }, "Core i5 (Kaby Lake)",         "14+ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Core(TM) i3-7###",          8 }, "Core i3 (Kaby Lake)",         "14+ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Pentium(R) G4###",          6 }, "Pentium (Kaby Lake)",         "14+ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Celeron(R) G3###",          6 }, "Celeron (Kaby Lake)",         "14+ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i7-7###X",        10 }, "Core i7 (Kaby Lake-X)",       "14+ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-7###X",        10 }, "Core i5 (Kaby Lake-X)",       "14+ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) i7-7.##",          8 }, "Core i7 (Kaby Lake-U)",       "14+ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) i5-7.##",          8 }, "Core i5 (Kaby Lake-U)",       "14+ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) i3-7.##",          8 }, "Core i3 (Kaby Lake-U)",       "14+ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) m3-7.##",          8 }, "Core m3 (Kaby Lake-U)",       "14+ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Pentium(R) 441#[UY]",       8 }, "Pentium Gold (Kaby Lake-U)",  "14+ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Celeron(R) 3###[UY]",       6 }, "Celeron (Kaby Lake-U)",       "14+ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i7-8###G",        10 }, "Core i7 (Kaby Lake-G)",       "14+ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-8###G",        10 }, "Core i5 (Kaby Lake-G)",       "14+ nm" },
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i7-8##0U",        10 }, "Core i7 (Kaby Lake-R)",       "14+ nm" }, /* i7-8550U + i7-8650U */
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i5-8##0U",        10 }, "Core i5 (Kaby Lake-R)",       "14+ nm" }, /* i5-8250U + i5-8350U */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) i3-8##0U",        10 }, "Core i3 (Kaby Lake-R)",       "14+ nm" }, /* i3-8130U */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Pentium(R) 4###U",          6 }, "Pentium Gold (Kaby Lake-R)",  "14+ nm" }, /* Pentium 4417U */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Celeron(R) 3###U",          6 }, "Celeron (Kaby Lake-R)",       "14+ nm" }, /* Celeron 3867U */
	/* Coffee Lake CPUs (2017, 8th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Coffee_Lake */
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Core(TM) i7-8###",       8 }, "Core i7 (Coffee Lake-S)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Core(TM) i5-8###",       8 }, "Core i5 (Coffee Lake-S)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i3-8###",       8 }, "Core i3 (Coffee Lake-S)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Pentium(R) G5###",       6 }, "Pentium Gold (Coffee Lake-S)",  "14++ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Celeron(R) G4###",       6 }, "Celeron (Coffee Lake-S)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Xeon(R) E-21##M",        8 }, "Xeon E (Coffee Lake-H)",        "14++ nm" },
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Core(TM) i9-8###[HB]",  10 }, "Core i9 (Coffee Lake-H)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Core(TM) i7-8###[HB]",  10 }, "Core i7 (Coffee Lake-H)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-8###[HB]",  10 }, "Core i5 (Coffee Lake-H)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-8###[HB]",  10 }, "Core i5 (Coffee Lake-H)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i3-8###[HB]",  10 }, "Core i3 (Coffee Lake-H)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i7-8###U",     10 }, "Core i7 (Coffee Lake-U)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-8###U",     10 }, "Core i5 (Coffee Lake-U)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Core(TM) i3-8###U",     10 }, "Core i3 (Coffee Lake-U)",       "14++ nm" },
	/* Coffee Lake Refresh CPUs (2018, 9th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Coffee_Lake#List_of_9th_generation_Coffee_Lake_processors_(Coffee_Lake_Refresh) */
	{  6, 14, -1, -1, 158,  8,    -1,    -1, { "Xeon(R) E-2###",         6 }, "Xeon E (Coffee Lake-S WS)",       "14++ nm" },
	{  6, 14, -1, -1, 158,  8,    -1,    -1, { "CC###",                  4 }, "CC (Coffee Lake)",                "14++ nm" }, /* CC150 */
	{  6, 14, -1, -1, 158,  8,    -1,    -1, { "Core(TM) i9-9###",       8 }, "Core i9 (Coffee Lake-S)",         "14++ nm" },
	{  6, 14, -1, -1, 158,  8,    -1,    -1, { "Core(TM) i7-9###",       8 }, "Core i7 (Coffee Lake-S)",         "14++ nm" },
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Core(TM) i5-9###",       8 }, "Core i5 (Coffee Lake-S)",         "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i3-9###",       8 }, "Core i3 (Coffee Lake-S)",         "14++ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Pentium(R) Gold G5###",  8 }, "Pentium Gold (Coffee Lake-S)",    "14++ nm" },
	{  6, 14, -1, -1, 158,  2,    -1,    -1, { "Celeron(R) G4###",       6 }, "Celeron (Coffee Lake-S)",         "14++ nm" },
	{  6, 14, -1, -1, 158, -1,    -1,    -1, { "Xeon(R) E-22##M",        8 }, "Xeon E (Coffee Lake-H Refresh)",  "14++ nm" },
	{  6, 14, -1, -1, 158,  8,    -1,    -1, { "Core(TM) i9-9###H",     10 }, "Core i9 (Coffee Lake-H Refresh)", "14++ nm" },
	{  6, 14, -1, -1, 158,  6,    -1,    -1, { "Core(TM) i7-9###H",     10 }, "Core i7 (Coffee Lake-H Refresh)", "14++ nm" },
	{  6, 14, -1, -1, 158,  4,    -1,    -1, { "Core(TM) i5-9###H",     10 }, "Core i5 (Coffee Lake-H Refresh)", "14++ nm" },
	/* Whiskey Lake CPUs (2018, 8th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Whiskey_Lake */
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i7-8##5U",     10 }, "Core i7 (Whiskey Lake-U)",      "14++ nm" },
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i5-8##5U",     10 }, "Core i5 (Whiskey Lake-U)",      "14++ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) i3-8##5U",     10 }, "Core i3 (Whiskey Lake-U)",      "14++ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Pentium(R) 5###U",       6 }, "Pentium Gold (Whiskey Lake-U)", "14++ nm" },
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Celeron(R) 4###U",       6 }, "Celeron (Whiskey Lake-U)",      "14++ nm" },
	/* Amber Lake CPUs (2018, 8th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Kaby_Lake#Amber_Lake */
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i7-8###Y",        10 }, "Core i7 (Amber Lake-Y)",      "14+ nm" }, /* i7-8500Y */
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i5-8###Y",        10 }, "Core i5 (Amber Lake-Y)",      "14+ nm" }, /* i5-8200Y + i5-82010Y + i5-8310Y */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) m3-8###Y",        10 }, "Core m3 (Amber Lake-Y)",      "14+ nm" }, /* m3-8100Y */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Pentium(R) 442#Y",          8 }, "Pentium Gold (Amber Lake-Y)", "14+ nm" }, /* Pentium 4425Y */
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i7-10###Y",       10 }, "Core i7 (Amber Lake-Y)",      "14+ nm" }, /* i7-10510Y */
	{  6, 14, -1, -1, 142,  4,    -1,    -1, { "Core(TM) i5-10###Y",       10 }, "Core i5 (Amber Lake-Y)",      "14+ nm" }, /* i5-10210Y + i5-10310Y + i5-8310Y */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Core(TM) i3-10###Y",       10 }, "Core i3 (Amber Lake-Y)",      "14+ nm" }, /* i3-10100Y + i3-10110Y */
	{  6, 14, -1, -1, 142,  2,    -1,    -1, { "Pentium(R) 65##Y",          6 }, "Pentium Gold (Amber Lake-Y)", "14+ nm" }, /* Pentium 6500Y */
	/* Cascade Lake CPUs (2019, 2nd Xeon Scalable gen, 14++ nm): https://en.wikichip.org/wiki/intel/microarchitectures/cascade_lake */
	{  6,  5,  7, -1, 85,  -1,    -1,    -1, { "Core(TM) i9-10###X",    10 }, "Core i9 (Cascade Lake-X)",        "14++ nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) W-#[23]##",      6 }, "Xeon W (Cascade Lake-W)",         "14++ nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Platinum #2##",  6 }, "Xeon Platinum (Cascade Lake-SP)", "14++ nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Gold #2##",      6 }, "Xeon Gold (Cascade Lake-SP)",     "14++ nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Silver #2##",    6 }, "Xeon Silver (Cascade Lake-SP)",   "14++ nm" },
	{  6,  5, -1, -1, 85,  -1,    -1,    -1, { "Xeon(R) Bronze #2##",    6 }, "Xeon Bronze (Cascade Lake-SP)",   "14++ nm" },
	/* Comet Lake CPUs (2019, 10th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Comet_Lake */
	{  6,  5, -1, -1, 165, -1,    -1,    -1, { "Xeon(R) W-12##",         6 }, "Xeon W (Comet Lake-S)",       "14++ nm" },
	{  6,  5, -1, -1, 165, 10,    -1,    -1, { "Core(TM) i9-10###",      8 }, "Core i9 (Comet Lake-S)",      "14++ nm" },
	{  6,  5, -1, -1, 165,  8,    -1,    -1, { "Core(TM) i7-10###",      8 }, "Core i7 (Comet Lake-S)",      "14++ nm" },
	{  6,  5, -1, -1, 165,  6,    -1,    -1, { "Core(TM) i5-10###",      8 }, "Core i5 (Comet Lake-S)",      "14++ nm" },
	{  6,  5, -1, -1, 165,  4,    -1,    -1, { "Core(TM) i3-10###",      8 }, "Core i3 (Comet Lake-S)",      "14++ nm" },
	{  6,  5, -1, -1, 165,  2,    -1,    -1, { "Pentium(R) Gold G6###",  8 }, "Pentium Gold (Comet Lake-S)", "14++ nm" },
	{  6,  5, -1, -1, 165,  2,    -1,    -1, { "Celeron(R) G5###",       6 }, "Celeron (Comet Lake-S)",      "14++ nm" },
	{  6,  6, -1, -1, 166,  6,    -1,    -1, { "Core(TM) i7-10###U",    10 }, "Core i7 (Comet Lake-U)",      "14++ nm" },
	{  6, 14, 12, -1, 142,  6,    -1,    -1, { "Core(TM) i7-10###U",    10 }, "Core i7 (Comet Lake-U)",      "14++ nm" },
	{  6, 14, 12, -1, 142,  4,    -1,    -1, { "Core(TM) i7-10###U",    10 }, "Core i7 (Comet Lake-U)",      "14++ nm" },
	{  6, 14, 12, -1, 142,  4,    -1,    -1, { "Core(TM) i5-10###U",    10 }, "Core i5 (Comet Lake-U)",      "14++ nm" },
	{  6, 14, 12, -1, 142,  4,    -1,    -1, { "Core(TM) i3-10###U",    10 }, "Core i3 (Comet Lake-U)",      "14++ nm" },
	{  6, 14, 12, -1, 142,  2,    -1,    -1, { "Pentium(R) Gold 6###U",  8 }, "Pentium Gold (Comet Lake-U)", "14++ nm" },
	{  6, 14, 12, -1, 142,  2,    -1,    -1, { "Celeron(R) 5###U",       6 }, "Celeron (Comet Lake-U)",      "14++ nm" },
	{  6,  5, -1, -1, 165, -1,    -1,    -1, { "Xeon(R) W-10###M",      10 }, "Xeon W (Comet Lake-H)",       "14++ nm" },
	{  6,  5, -1, -1, 165, -1,    -1,    -1, { "Core(TM) i9-10###H",    10 }, "Core i9 (Comet Lake-H)",      "14++ nm" },
	{  6,  5, -1, -1, 165, -1,    -1,    -1, { "Core(TM) i7-10###H",    10 }, "Core i7 (Comet Lake-H)",      "14++ nm" },
	{  6,  5, -1, -1, 165, -1,    -1,    -1, { "Core(TM) i5-10###H",    10 }, "Core i5 (Comet Lake-H)",      "14++ nm" },

	/* Goldmont CPUs (2016, Atom, 14 nm): https://en.wikipedia.org/wiki/Goldmont */
	{  6, 12, -1, -1,  92, -1,    -1,    -1, { "Pentium(R) J4###",  6 }, "Pentium (Apollo Lake)", "14 nm" },
	{  6, 12, -1, -1,  92, -1,    -1,    -1, { "Celeron(R) J3###",  6 }, "Celeron (Apollo Lake)", "14 nm" },
	{  6, 12, -1, -1,  92, -1,    -1,    -1, { "Pentium(R) N4###",  6 }, "Pentium (Apollo Lake)", "14 nm" },
	{  6, 12, -1, -1,  92, -1,    -1,    -1, { "Celeron(R) N3###",  6 }, "Celeron (Apollo Lake)", "14 nm" },
	{  6, 12, -1, -1,  92, -1,    -1,    -1, { "Atom(TM) E39##",    6 }, "Atom (Apollo Lake)",    "14 nm" },
	{  6, 15, -1, -1,  95, -1,    -1,    -1, { "Atom(TM) C39##",    6 }, "Atom (Denverton)" ,     "14 nm" },

	/* Goldmont Plus CPUs (2017, Atom, 14 nm): https://en.wikipedia.org/wiki/Goldmont_Plus */
	{  6, 10, -1, -1, 122, -1,    -1,    -1, { "Pentium(R) Silver [JN]5###",  8 }, "Pentium Silver (Gemini Lake)", "14 nm" },
	{  6, 10, -1, -1, 122, -1,    -1,    -1, { "Celeron(R) [JN]4###",         6 }, "Celeron (Gemini Lake)",        "14 nm" },

	/* Palm Cove CPUs (2018, 8th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Cannon_Lake_(microprocessor)*/
	{  6,  6, -1, -1, 102,  2,    -1,    -1, { "Core(TM) i3-8###U",     10 }, "Core i3 (Cannon Lake-U)",       "14++ nm" }, /* Core i3 8121U */
	{  6,  6, -1, -1, 102,  2,    -1,    -1, { "Core(TM) m3-8###Y",     10 }, "Core m3 (Cannon Lake-Y)",       "14++ nm" }, /* Core m3 8114Y */

	/* Sunny Cove CPUs (2019, 10th Core i gen, 10 nm): https://en.wikipedia.org/wiki/Sunny_Cove_(microarchitecture) */
	{  6, 14, -1, -1, 126,  4,    -1,    -1, { "Core(TM) i7-10##NG7",    10 }, "Core i7 (Ice Lake)", "10 nm" },
	{  6, 14, -1, -1, 126,  4,    -1,    -1, { "Core(TM) i7-10##G7",     10 }, "Core i7 (Ice Lake)", "10 nm" },
	{  6, 14, -1, -1, 126,  4,    -1,    -1, { "Core(TM) i5-10##NG7",    10 }, "Core i5 (Ice Lake)", "10 nm" },
	{  6, 14, -1, -1, 126,  4,    -1,    -1, { "Core(TM) i5-10##G[741]", 10 }, "Core i5 (Ice Lake)", "10 nm" },
	{  6, 14, -1, -1, 126,  2,    -1,    -1, { "Core(TM) i3-10##G[14]",  10 }, "Core i3 (Ice Lake)", "10 nm" },
	{  6, 14, -1, -1, 126,  2,    -1,    -1, { "Core(TM) i3-10##NG4",    10 }, "Core i3 (Ice Lake)", "10 nm" },
	{  6, 14, -1, -1, 126,  2,    -1,    -1, { "Pentium(R) 68##",         4 }, "Pentium (Ice Lake)", "10 nm" }, /* Pentium 6805 */
	/* Ice Lake (server) CPUs (2021, 3rd Xeon Scalable gen, 10 nm): https://en.wikichip.org/wiki/intel/microarchitectures/ice_lake_(server) */
	{  6, 12, -1, -1, 108,  -1,   -1,    -1, { "Xeon(R) D-[12]7##",      6 }, "Xeon D (Ice Lake-D)",         "10 nm" },
	{  6, 10, -1, -1, 106,  -1,   -1,    -1, { "Xeon(R) W-#3##",         6 }, "Xeon W (Ice Lake-W)",         "10 nm" },
	{  6, 10, -1, -1, 106,  -1,   -1,    -1, { "Xeon(R) Platinum #3##",  6 }, "Xeon Platinum (Ice Lake-SP)", "10 nm" },
	{  6, 10, -1, -1, 106,  -1,   -1,    -1, { "Xeon(R) Gold #3##",      6 }, "Xeon Gold (Ice Lake-SP)",     "10 nm" },
	{  6, 10, -1, -1, 106,  -1,   -1,    -1, { "Xeon(R) Silver #3##",    6 }, "Xeon Silver (Ice Lake-SP)",   "10 nm" },
	{  6, 10, -1, -1, 106,  -1,   -1,    -1, { "Xeon(R) Bronze #3##",    6 }, "Xeon Bronze (Ice Lake-SP)",   "10 nm" },

	/* Tremont CPUs (2020, Atom, 10 nm): https://en.wikipedia.org/wiki/Tremont_(microarchitecture) */
	{  6,  6, -1, -1, 150, -1,    -1,    -1, { "Pentium(R) [JN]6###",      6 }, "Pentium (Elkhart Lake)",       "10 nm" },
	{  6,  6, -1, -1, 150, -1,    -1,    -1, { "Celeron(R) [JN]6###",      6 }, "Celeron (Elkhart Lake)",       "10 nm" },
	{  6,  6, -1, -1, 150, -1,    -1,    -1, { "Atom(TM) x6###",           6 }, "Atom (Elkhart Lake)",          "10 nm" },
	{  6, 10, -1, -1, 138, -1,    -1,    -1, { "Core(TM) i5-L##G7",       12 }, "Core i5 (Lakefield)",          "10 nm" },
	{  6, 10, -1, -1, 138, -1,    -1,    -1, { "Core(TM) i3-L##G4",       12 }, "Core i3 (Lakefield)",          "10 nm" },
	{  6, 12, -1, -1, 156, -1,    -1,    -1, { "Pentium(R) Silver N6###",  8 }, "Pentium Silver (Jasper Lake)", "10 nm" },
	{  6, 12, -1, -1, 156, -1,    -1,    -1, { "Celeron(R) N[45]###",      6 }, "Celeron (Jasper Lake)",        "10 nm" },

	/* Willow Cove CPUs (2020, 11th Core i gen, 10 nm SuperFin): https://en.wikipedia.org/wiki/Willow_Cove */
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i7-11#5G7",   12 }, "Core i7 (Tiger Lake-UP3)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i5-11#5G7",   12 }, "Core i5 (Tiger Lake-UP3)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i3-11#5G4",   12 }, "Core i3 (Tiger Lake-UP3)",      "10SF" },
	{  6, 12, -1, -1, 140,  2,    -1,    -1, { "Pentium(R) Gold 7##5",  6 }, "Pentium Gold (Tiger Lake-UP3)", "10SF" },
	{  6, 12, -1, -1, 140,  2,    -1,    -1, { "Celeron(R) 6##5",       4 }, "Celeron (Tiger Lake-UP3)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i7-11#0G7",   12 }, "Core i7 (Tiger Lake-UP4)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i5-11#0G7",   12 }, "Core i5 (Tiger Lake-UP4)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i3-11#0G4",   12 }, "Core i3 (Tiger Lake-UP4)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i7-11###H",   10 }, "Core i7 (Tiger Lake-H35)",      "10SF" },
	{  6, 12, -1, -1, 140, -1,    -1,    -1, { "Core(TM) i5-11###H",   10 }, "Core i5 (Tiger Lake-H35)",      "10SF" },
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Xeon(R) W-11###M",     10 }, "Xeon W (Tiger Lake-H)",         "10SF" },
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i9-11###H",   10 }, "Core i9 (Tiger Lake-H)",        "10SF" },
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i7-11###H",   10 }, "Core i7 (Tiger Lake-H)",        "10SF" },
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i5-11###H",   10 }, "Core i5 (Tiger Lake-H)",        "10SF" },
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i9-11###KB",  12 }, "Core i9 (Tiger Lake-B)",        "10SF" }, /* i9-11900KB */
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i7-11###B",   10 }, "Core i7 (Tiger Lake-B)",        "10SF" }, /* i7-11700B */
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i5-11###B",   10 }, "Core i5 (Tiger Lake-B)",        "10SF" }, /* i5-11500B */
	{  6, 13, -1, -1, 141, -1,    -1,    -1, { "Core(TM) i5-11###B",   10 }, "Core i5 (Tiger Lake-B)",        "10SF" }, /* i3-11100B */

	/* Cypress Cove CPUs (2021, 11th Core i gen, 14++ nm): https://en.wikipedia.org/wiki/Sunny_Cove_(microarchitecture)#Cypress_Cove */
	{  6, 7, -1, -1, 167,  -1,    -1,    -1, { "Core(TM) i9-11###",  8 }, "Core i9 (Rocket Lake-S)", "14++ nm" },
	{  6, 7, -1, -1, 167,  -1,    -1,    -1, { "Core(TM) i7-11###",  8 }, "Core i7 (Rocket Lake-S)", "14++ nm" },
	{  6, 7, -1, -1, 167,  -1,    -1,    -1, { "Core(TM) i5-11###",  8 }, "Core i5 (Rocket Lake-S)", "14++ nm" },
	{  6, 7, -1, -1, 167,  -1,    -1,    -1, { "Xeon(R) E-23##",     6 }, "Xeon E (Rocket Lake)"   , "14++ nm" },

	/* Golden Cove (P-cores) / Gracemont (E-cores) CPUs (2021, 12th Core i gen, Intel 7): https://en.wikipedia.org/wiki/Golden_Cove */
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i9-12###",      8 }, "Core i9 (Alder Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i7-12###",      8 }, "Core i7 (Alder Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i5-12###",      8 }, "Core i5 (Alder Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i3-12###",      8 }, "Core i3 (Alder Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Pentium(R) Gold G7###",  8 }, "Pentium Gold (Alder Lake-S)"   , "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Celeron(R) G6###",       6 }, "Celeron (Alder Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i9-12###HX",   12 }, "Core i9 (Alder Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i7-12###HX",   12 }, "Core i7 (Alder Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 151, -1,    -1,    -1, { "Core(TM) i5-12###HX",   12 }, "Core i5 (Alder Lake-HX)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i7-12##P",     10 }, "Core i7 (Alder Lake-P)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i5-12##P",     10 }, "Core i5 (Alder Lake-P)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i3-12##P",     10 }, "Core i3 (Alder Lake-P)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i7-12##U",     10 }, "Core i7 (Alder Lake-U)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i5-12##U",     10 }, "Core i5 (Alder Lake-U)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i3-12##U",     10 }, "Core i3 (Alder Lake-U)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Pentium(R) Gold 8###",   6 }, "Pentium Gold (Alder Lake-U)",    "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Celeron(R) 7###",        4 }, "Celeron (Alder Lake-U)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i9-12###H",    10 }, "Core i9 (Alder Lake-H)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i7-12###H",    10 }, "Core i7 (Alder Lake-H)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i5-12###H",    10 }, "Core i5 (Alder Lake-H)",         "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i7-12##UL",    12 }, "Core i7 (Alder Lake-PS)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i5-12##UL",    12 }, "Core i5 (Alder Lake-PS)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i3-12##UL",    12 }, "Core i3 (Alder Lake-PS)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Celeron(R) 7###L",       6 }, "Celeron (Alder Lake-PS)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i7-12###HL",   12 }, "Core i7 (Alder Lake-PS)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i5-12###HL",   12 }, "Core i5 (Alder Lake-PS)",        "Intel 7" },
	{  6, 10, -1, -1, 154, -1,    -1,    -1, { "Core(TM) i3-12###HL",   12 }, "Core i3 (Alder Lake-PS)",        "Intel 7" },
	/* Sapphire Rapids CPUs (2023, 4th Xeon Scalable gen, Intel 7): https://en.wikichip.org/wiki/intel/microarchitectures/sapphire_rapids */
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) w9-#4##",        6 }, "Xeon w9 (Sapphire Rapids-WS)",       "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) w7-#4##",        6 }, "Xeon w7 (Sapphire Rapids-WS)",       "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) w5-#4##",        6 }, "Xeon w5 (Sapphire Rapids-WS)",       "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) w3-#4##",        6 }, "Xeon w3 (Sapphire Rapids-WS)",       "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) Max #4##",       6 }, "Xeon Max (Sapphire Rapids-HBM)",     "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) Platinum #4##",  6 }, "Xeon Platinum (Sapphire Rapids-SP)", "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) Gold #4##",      6 }, "Xeon Gold (Sapphire Rapids-SP)",     "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) Silver #4##",    6 }, "Xeon Silver (Sapphire Rapids-SP)"  , "Intel 7" },
	{  6, 15, -1, -1, 143, -1,    -1,    -1, { "Xeon(R) Bronze #4##",    6 }, "Xeon Bronze (Sapphire Rapids-SP)"  , "Intel 7" },

	/* Gracemont CPUs (2021, Atom, Intel 7): https://en.wikipedia.org/wiki/Gracemont_(microarchitecture) */
	{  6, 14, -1, -1, 190, -1,    -1,    -1, { "Core(TM) i3-N3##",      10 }, "Core i3 (Alder Lake-N)",         "Intel 7" }, /* Core i3 N300 + Core i3 N305 */
	{  6, 14, -1, -1, 190,  4,    -1,    -1, { "N##",                    2 }, "Intel Processor (Alder Lake-N)", "Intel 7" },
	{  6, 14, -1, -1, 190,  2,    -1,    -1, { "N##",                    2 }, "Intel Processor (Alder Lake-N)", "Intel 7" }, /* Intel Processor N50 */
	{  6, 14, -1, -1, 190, -1,    -1,    -1, { "Atom(TM) x7###E",        8 }, "Atom (Alder Lake-N)",            "Intel 7" },
	/* Twin Lake CPUs (2025, Atom, Intel 7): https://en.wikichip.org/wiki/intel/microarchitectures/twin_lake */
	{  6, 14, -1, -1, 190,  8,    -1,    -1, { "Core(TM) 3 N#5#",        8 }, "Core 3 (Twin Lake-N)",            "Intel 7" }, /* Core 3 N350 + Core 3 N355 */
	{  6, 14, -1, -1, 190,  4,    -1,    -1, { "N#5#",                   4 }, "Intel Processor (Twin Lake-N)",   "Intel 7" }, /* Intel Processor N150 + Intel Processor N150 */

	/* Raptor Cove (P-cores) / Gracemont (E-cores) CPUs (2022, 13th Core i gen, Intel 7): https://en.wikipedia.org/wiki/Golden_Cove#Raptor_Cove */
	{  6, 15, -1, -1, 191, -1,    -1,    -1, { "Core(TM) i5-13###",      8 }, "Core i5 (Raptor Lake-S)",         "Intel 7" }, /* "Golden Cove" cores */
	{  6, 15, -1, -1, 191, -1,    -1,    -1, { "Core(TM) i3-13###",      8 }, "Core i3 (Raptor Lake-S)",         "Intel 7" }, /* "Golden Cove" cores */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i9-13###",      8 }, "Core i9 (Raptor Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i7-13###",      8 }, "Core i7 (Raptor Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i5-13###",      8 }, "Core i5 (Raptor Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i3-13###",      8 }, "Core i3 (Raptor Lake-S)",         "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i9-13###HX",   12 }, "Core i9 (Raptor Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i7-13###HX",   12 }, "Core i7 (Raptor Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i5-13###HX",   12 }, "Core i5 (Raptor Lake-HX)",        "Intel 7" },
	{  6, 10,  2, -1, 186, -1,    -1,    -1, { "Core(TM) i7-13###P",    10 }, "Core i7 (Raptor Lake-P)",         "Intel 7" },
	{  6, 10,  2, -1, 186, -1,    -1,    -1, { "Core(TM) i5-13###P",    10 }, "Core i5 (Raptor Lake-P)",         "Intel 7" },
	{  6, 10,  3, -1, 186, -1,    -1,    -1, { "Core(TM) i7-13###U",    10 }, "Core i7 (Raptor Lake-U)",         "Intel 7" },
	{  6, 10,  3, -1, 186, -1,    -1,    -1, { "Core(TM) i5-13###U",    10 }, "Core i5 (Raptor Lake-U)",         "Intel 7" },
	{  6, 10,  3, -1, 186, -1,    -1,    -1, { "Core(TM) i3-13###U",    10 }, "Core i3 (Raptor Lake-U)",         "Intel 7" },
	{  6, 10,  3, -1, 186, -1,    -1,    -1, { "U300",                   4 }, "Intel Processor (Raptor Lake-U)", "Intel 7" }, /* Intel Processor U300 */
	{  6, 10, -1, -1, 186, -1,    -1,    -1, { "Core(TM) i9-13###H",    10 }, "Core i9 (Raptor Lake-H)",         "Intel 7" },
	{  6, 10, -1, -1, 186, -1,    -1,    -1, { "Core(TM) i7-13###H",    10 }, "Core i7 (Raptor Lake-H)",         "Intel 7" },
	{  6, 10, -1, -1, 186, -1,    -1,    -1, { "Core(TM) i5-13###H",    10 }, "Core i5 (Raptor Lake-H)",         "Intel 7" },
	/* Emerald Rapids CPUs (2023, 5th Xeon Scalable gen, Intel 7): https://en.wikichip.org/wiki/intel/microarchitectures/emerald_rapids */
	{  6, 15, -1, -1, 207, -1,    -1,    -1, { "Xeon(R) Platinum #5##",  6 }, "Xeon Platinum (Emerald Rapids-SP)", "Intel 7" }, /* Xeon Platinum (8500) */
	{  6, 15, -1, -1, 207, -1,    -1,    -1, { "Xeon(R) Gold #5##",      6 }, "Xeon Gold (Emerald Rapids-SP)",     "Intel 7" }, /* Xeon Gold (5500 and 6500) */
	{  6, 15, -1, -1, 207, -1,    -1,    -1, { "Xeon(R) Silver #5##",    6 }, "Xeon Silver (Emerald Rapids-SP)",   "Intel 7" }, /* Xeon Silver (4500) */
	{  6, 15, -1, -1, 207, -1,    -1,    -1, { "Xeon(R) Bronze #5##",    6 }, "Xeon Bronze (Emerald Rapids-SP)",   "Intel 7" }, /* Xeon Bronze (3500) */
	/* Raptor Lake Refresh CPUs (2023, 14th Core i gen, Intel 7): https://en.wikipedia.org/wiki/Raptor_Lake#List_of_14th_generation_Raptor_Lake_processors */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i9-14###",      8 }, "Core i9 (Raptor Lake-S)" ,        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i7-14###",      8 }, "Core i7 (Raptor Lake-S)" ,        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i5-14###",      8 }, "Core i5 (Raptor Lake-S)" ,        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i3-14###",      8 }, "Core i3 (Raptor Lake-S)" ,        "Intel 7" },
	{  6,  7, -1, -1, 183,  2,    -1,    -1, { "300",                    2 }, "Intel Processor (Raptor Lake-S)", "Intel 7" }, /* Intel Processor 300 + Intel Processor 300T */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i9-14###HX",   12 }, "Core i9 (Raptor Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i7-14###HX",   12 }, "Core i7 (Raptor Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) i5-14###HX",   12 }, "Core i5 (Raptor Lake-HX)",        "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Xeon(R) E-24##",         6 }, "Xeon E (Raptor Lake)",            "Intel 7" },
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 7 1##U",        8 }, "Core 7 (Raptor Lake-U)",          "Intel 7" }, /* Core 7 150U */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 5 1##U",        8 }, "Core 5 (Raptor Lake-U)",          "Intel 7" }, /* Core 5 120U */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 3 1##U",        8 }, "Core 3 (Raptor Lake-U)",          "Intel 7" }, /* Core 3 100U */
	/* Raptor Lake Re-refresh CPUs (2025, Core Series 2, Intel 7): https://en.wikipedia.org/wiki/Raptor_Lake#List_of_Core_Series_2_processors */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 7 2##U",        8 }, "Core 7 (Raptor Lake-U)",          "Intel 7" }, /* Core 7 250U */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 5 2##U",        8 }, "Core 5 (Raptor Lake-U)",          "Intel 7" }, /* Core 5 220U */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 9 2##H",        8 }, "Core 9 (Raptor Lake-H)",          "Intel 7" }, /* Core 9 270H */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 7 2##H",        8 }, "Core 7 (Raptor Lake-H)",          "Intel 7" }, /* Core 7 240H + Core 7 250H */
	{  6,  7, -1, -1, 183, -1,    -1,    -1, { "Core(TM) 5 2##H",        8 }, "Core 5 (Raptor Lake-H)",          "Intel 7" }, /* Core 5 210H + Core 5 220H */

	/* Redwood Cove (P-cores) / Crestmont (E-cores) CPUs (2023, Core Ultra Series 1, Intel 4): https://en.wikipedia.org/wiki/Meteor_Lake */
	{  6, 10, -1, -1, 170, -1,    -1,    -1, { "Core(TM) Ultra 9 1##H", 10 }, "Core Ultra 9 (Meteor Lake-H)", "Intel 4" },
	{  6, 10, -1, -1, 170, -1,    -1,    -1, { "Core(TM) Ultra 7 1##H", 10 }, "Core Ultra 7 (Meteor Lake-H)", "Intel 4" },
	{  6, 10, -1, -1, 170, -1,    -1,    -1, { "Core(TM) Ultra 5 1##H", 10 }, "Core Ultra 5 (Meteor Lake-H)", "Intel 4" },
	{  6, 10, -1, -1, 170, -1,    -1,    -1, { "Core(TM) Ultra 7 1##U", 10 }, "Core Ultra 7 (Meteor Lake-U)", "Intel 4" },
	{  6, 10, -1, -1, 170, -1,    -1,    -1, { "Core(TM) Ultra 5 1##U", 10 }, "Core Ultra 5 (Meteor Lake-U)", "Intel 4" },
	/* Granite Rapids CPUs (2024, 6th Xeon Scalable gen, Intel 7): https://en.wikipedia.org/wiki/Granite_Rapids */
	{  6, 13, -1, -1, 173, -1,    -1,    -1, { "Xeon(R) 6[57]##P",  6 }, "Xeon 6 (Granite Rapids-SP)",  "Intel 3" },
	//{  6, ??, -1, -1, ???, -1,    -1,    -1, { "Xeon(R) 6[57]##P",  6 }, "Xeon 6 (Granite Rapids-AP)",  "Intel 3" },
	//{  6, 14, -1, -1, 174, -1,    -1,    -1, { "Xeon(R) ????",      4 }, "Xeon ??? (Granite Rapids-D)", "Intel 3" },

	/* Lion Cove (P-cores) / Skymont (E-cores) CPUs (2024, Core Ultra Series 2, TSMC N3B): https://en.wikipedia.org/wiki/Arrow_Lake_(microprocessor) */
	{  6,  6, -1, -1, 198, -1,    -1,    -1, { "Core(TM) Ultra 9 2##",    8 }, "Core Ultra 9 (Arrow Lake-S)",  "TSMC N3B" },
	{  6,  6, -1, -1, 198, -1,    -1,    -1, { "Core(TM) Ultra 7 2##",    8 }, "Core Ultra 7 (Arrow Lake-S)",  "TSMC N3B" },
	{  6,  6, -1, -1, 198, -1,    -1,    -1, { "Core(TM) Ultra 5 2##",    8 }, "Core Ultra 5 (Arrow Lake-S)",  "TSMC N3B" },
	{  6,  6, -1, -1, 181, -1,    -1,    -1, { "Core(TM) Ultra 7 2##U",  10 }, "Core Ultra 7 (Arrow Lake-U)",  "TSMC N3B" }, /* Core Ultra 7 255U + Core Ultra 7 265U */
	{  6,  6, -1, -1, 181, -1,    -1,    -1, { "Core(TM) Ultra 5 2##U",  10 }, "Core Ultra 5 (Arrow Lake-U)",  "TSMC N3B" }, /* Core Ultra 5 225U + Core Ultra 5 235U */
	{  6,  6, -1, -1, 197, -1,    -1,    -1, { "Core(TM) Ultra 9 2##H",  10 }, "Core Ultra 9 (Arrow Lake-H)",  "TSMC N3B" }, /* Core Ultra 9 285H */
	{  6,  6, -1, -1, 197, -1,    -1,    -1, { "Core(TM) Ultra 7 2##H",  10 }, "Core Ultra 7 (Arrow Lake-H)",  "TSMC N3B" }, /* Core Ultra 7 255H + Core Ultra 7 265H */
	{  6,  6, -1, -1, 197, -1,    -1,    -1, { "Core(TM) Ultra 5 2##H",  10 }, "Core Ultra 5 (Arrow Lake-H)",  "TSMC N3B" }, /* Core Ultra 5 225H + Core Ultra 5 235H */
	{  6,  6, -1, -1, 197, -1,    -1,    -1, { "Core(TM) Ultra 9 2##HX", 12 }, "Core Ultra 9 (Arrow Lake-HX)", "TSMC N3B" }, /* Core Ultra 9 275HX + Core Ultra 9 285HX */
	{  6,  6, -1, -1, 197, -1,    -1,    -1, { "Core(TM) Ultra 7 2##HX", 12 }, "Core Ultra 7 (Arrow Lake-HX)", "TSMC N3B" }, /* Core Ultra 7 255HX + Core Ultra 7 265HX */
	{  6,  6, -1, -1, 197, -1,    -1,    -1, { "Core(TM) Ultra 5 2##HX", 12 }, "Core Ultra 5 (Arrow Lake-HX)", "TSMC N3B" }, /* Core Ultra 5 235HX + Core Ultra 5 245HX */
	{  6, 13, -1, -1, 189, -1,    -1,    -1, { "Core(TM) Ultra 9 2##V",  10 }, "Core Ultra 9 (Lunar Lake-V)",  "TSMC N3B" },
	{  6, 13, -1, -1, 189, -1,    -1,    -1, { "Core(TM) Ultra 7 2##V",  10 }, "Core Ultra 7 (Lunar Lake-V)",  "TSMC N3B" },
	{  6, 13, -1, -1, 189, -1,    -1,    -1, { "Core(TM) Ultra 5 2##V",  10 }, "Core Ultra 5 (Lunar Lake-V)",  "TSMC N3B" },

	/* Cougar Cove (P-cores) / Darkmont (E-cores and LP E-cores) CPUs (2025, Core Ultra Series 3, Intel 18A): https://en.wikipedia.org/wiki/Panther_Lake_(microprocessor) */
	// TBA
//     F   M   S  EF    EM #cores L2$    L3$ Pattern                          Codename                       Technology


	/* Itaniums */
	{  7, -1, -1, -1, -1,   1,    -1,    -1, { "",  0 }, "Itanium",   UNKN_STR },
	{ 15, -1, -1, 16, -1,   1,    -1,    -1, { "",  0 }, "Itanium 2", UNKN_STR },
};

// https://github.com/anrieff/libcpuid/blob/2e4456ae0165db3155da2e8fba92afd5c090ca1b/libcpuid/recog_amd.c
/*
 * Copyright 2008  Veselin Georgiev,
 * anrieffNOSPAM @ mgail_DOT.com (convert to gmail)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
 * Useful links:
 * - List of AMD CPU microarchitectures: https://en.wikipedia.org/wiki/List_of_AMD_CPU_microarchitectures
 * - List of AMD Athlon processors: https://en.wikipedia.org/wiki/List_of_AMD_Athlon_processors#Desktop_processors
 * - List of AMD Duron processors: https://en.wikipedia.org/wiki/List_of_AMD_Duron_processors
 * - List of AMD Sempron processors: https://en.wikipedia.org/wiki/List_of_AMD_Sempron_processors
 * - List of AMD Turion processors: https://en.wikipedia.org/wiki/List_of_AMD_Turion_processors
 * - List of AMD Opteron processors: https://en.wikipedia.org/wiki/List_of_AMD_Opteron_processors
 * - List of AMD Phenom processors: https://en.wikipedia.org/wiki/List_of_AMD_Phenom_processors
 * - List of AMD FX processors: https://en.wikipedia.org/wiki/List_of_AMD_FX_processors
 * - List of AMD processors with 3D graphics: https://en.wikipedia.org/wiki/List_of_AMD_processors_with_3D_graphics
 * - List of AMD Ryzen processors: https://en.wikipedia.org/wiki/List_of_AMD_Ryzen_processors
 * - List of AMD Epyc processors: https://en.wikipedia.org/wiki/Epyc#List_of_Epyc_processors
 * - Processor Specifications: https://www.amd.com/en/products/specifications/processors.html
 */
const struct match_entry_t cpudb_amd[] = {
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                          Codename                          Technology
	{ -1, -1, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown AMD CPU",                UNKN_STR },

	/* 486 and the likes */
	{  4, -1, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown 486",                    UNKN_STR },
	{  4,  3, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "AMD 486DX2",                     UNKN_STR },
	{  4,  7, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "AMD 486DX2WB",                   UNKN_STR },
	{  4,  8, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "AMD 486DX4",                     UNKN_STR },
	{  4,  9, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "AMD 486DX4WB",                   UNKN_STR },

	/* Pentia clones */
	{  5, -1, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown K5",                     UNKN_STR },
	{  5,  0, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "K5 (SSA/5)",                     UNKN_STR },
	{  5,  1, -1, -1,   -1,   1,    -1,    -1, { "K5(tm)",                 2 }, "K5 (5k86)",                      "350 nm" },
	{  5,  2, -1, -1,   -1,   1,    -1,    -1, { "K5(tm)",                 2 }, "K5 (5k86)",                      "350 nm" },
	{  5,  3, -1, -1,   -1,   1,    -1,    -1, { "K5(tm)",                 2 }, "K5 (5k86)",                      "350 nm" },

	/* K6 Architecture */
	{  5, -1, -1, -1,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown K6",                     UNKN_STR },
	{  5,  6, -1, -1,   -1,   1,    -1,    -1, { "K6",                     2 }, "K6",                             "350 nm" },
	{  5,  7, -1, -1,   -1,   1,    -1,    -1, { "K6",                     2 }, "K6 (Little Foot)",               "250 nm" },
	{  5,  8,  0, -1,   -1,   1,    -1,    -1, { "K6(tm)",                 2 }, "K6-2 (Chomper)",                 "250 nm" },
	{  5,  8, 12, -1,   -1,   1,    -1,    -1, { "K6(tm)",                 2 }, "K6-2 (Chomper Extended)",        "250 nm" },
	{  5,  9, -1, -1,   -1,   1,    -1,    -1, { "K6(tm)",                 2 }, "K6-III (Sharptooth)",            "250 nm" },
	{  5, 13, -1, -1,   -1,   1,    -1,    -1, { "K6(tm)",                 2 }, "K6-2+",                          "180 nm" },
	{  5, 13, -1, -1,   -1,   1,    -1,    -1, { "K6(tm)-III ",            4 }, "K6-III+",                        "180 nm" },

	/* K7 Architecture */
	{  6, -1, -1, 15,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown K7",                     UNKN_STR },

	{  6,  1, -1, -1,   -1,   1,    -1,    -1, { "K7(tm)",                 0 }, "Athlon (Argon)",                 "250 nm" },

	{  6,  2, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm)",             0 }, "Athlon (Pluto/Orion)",           "180 nm" },

	{  6,  3, -1, -1,   -1,   1,    -1,    -1, { "Duron(tm)",              2 }, "Duron (Spitfire)",               "180 nm" },
	{  6,  3, -1, -1,   -1,   1,    -1,    -1, { "Duron(tm) M",            4 }, "Mobile Duron (Spitfire)",        "180 nm" },

	{  6,  4, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm)",             2 }, "Athlon (ThunderBird)",           "180 nm" },

	{  6,  6, -1, -1,   -1,   1,    -1,    -1, { "Athlon XP",              4 }, "Athlon (Palomino)",              "180 nm" },
	{  6,  6, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm) MP",          4 }, "Athlon MP (Palomino)",           "180 nm" },
	{  6,  6, -1, -1,   -1,   1,    -1,    -1, { "Duron(tm)",              2 }, "Duron (Palomino)",               "180 nm" },

	{  6,  7, -1, -1,   -1,   1,    -1,    -1, { "Duron(tm)",              2 }, "Duron (Morgan)",                 "180 nm" },
	{  6,  7, -1, -1,   -1,   1,    -1,    -1, { "Duron(tm) M",            4 }, "Mobile Duron (Camaro)",          "180 nm" },

	{  6,  8, -1, -1,   -1,   1,    -1,    -1, { "Athlon",                 2 }, "Athlon XP (Thoroughbred)",       "130 nm" },
	{  6,  8, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm) XP",          4 }, "Athlon XP (Thoroughbred)",       "130 nm" },
	{  6,  8, -1, -1,   -1,   1,    -1,    -1, { "Duron(tm)",              2 }, "Duron (Applebred)",              "130 nm" },
	{  6,  8, -1, -1,   -1,   1,    -1,    -1, { "Sempron(tm)",            2 }, "Sempron (Thoroughbred)",         "130 nm" },
	{  6,  8, -1, -1,   -1,   1,   128,    -1, { "Sempron(tm)",            2 }, "Sempron (Thoroughbred)",         "130 nm" },
	{  6,  8, -1, -1,   -1,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron (Thoroughbred)",         "130 nm" },
	{  6,  8, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm) MP",          4 }, "Athlon MP (Thoroughbred)",       "130 nm" },
	{  6,  8, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm) XP-M",        6 }, "Mobile Athlon (Thoroughbred)",   "130 nm" },

	{  6, 10, -1, -1,   -1,   1,   512,    -1, { "Athlon(tm) XP",          4 }, "Athlon XP (Barton)",             "130 nm" },
	{  6, 10, -1, -1,   -1,   1,   512,    -1, { "Sempron(tm)",            2 }, "Sempron (Barton)",               "130 nm" },
	{  6, 10, -1, -1,   -1,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron (Thorton)",              "130 nm" },
	{  6, 10, -1, -1,   -1,   1,   256,    -1, { "Athlon(tm) XP",          4 }, "Athlon XP (Thorton)",            "130 nm" },
	{  6, 10, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm) MP",          4 }, "Athlon MP (Barton)",             "130 nm" },
	{  6, 10, -1, -1,   -1,   1,    -1,    -1, { "Athlon(tm) XP-M",        6 }, "Mobile Athlon (Barton)",         "130 nm" },

	/* K8 Architecture */
	{ 15, -1, -1, 15,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown K8",                     UNKN_STR },
	{ 15, -1, -1, 16,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown K9",                     UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,    -1,    -1, { "",                       0 }, "Unknown A64",                    UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,    -1,    -1, { "Opteron(tm)",            2 }, "Opteron",                        UNKN_STR },
	{ 15, -1, -1, 15,   -1,   2,    -1,    -1, { "Dual Core AMD Opteron",  8 }, "Opteron (Dual Core)",            UNKN_STR },
	{ 15,  1, -1, 15,   65,   2,    -1,    -1, { "Opteron(tm) 22##",       6 }, "Opteron (Santa Rosa)",           "90 nm" },
	{ 15,  3, -1, 15,   -1,   1,    -1,    -1, { "Opteron(tm)",            2 }, "Opteron",                        UNKN_STR },
	{ 15,  3, -1, 15,   -1,   2,    -1,    -1, { "Dual Core AMD Opteron",  8 }, "Opteron (Dual Core)",            UNKN_STR },
	{ 15,  5, -1, 15,    5,  -1,    -1,    -1, { "Opteron(tm) [128]##",    4 }, "Opteron (SledgeHammer)",         "130 nm" },
	{ 15, -1, -1, 15,   -1,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (512K)",               UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,  1024,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (1024K)",              UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,    -1,    -1, { "Athlon(tm) FX",          4 }, "Athlon FX",                      UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,    -1,    -1, { "Athlon(tm) 64 FX",       6 }, "Athlon 64 FX",                   UNKN_STR },
	{ 15,  3, -1, 15,   35,   2,    -1,    -1, { "Athlon(tm) 64 FX",       6 }, "Athlon 64 FX X2 (Toledo)",       "90 nm"  },
	{ 15, -1, -1, 15,   -1,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (512K)",            UNKN_STR },
	{ 15, -1, -1, 15,   -1,   2,  1024,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (1024K)",           UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,   512,    -1, { "Turion(tm) 64",          4 }, "Turion 64 (512K)",               UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,  1024,    -1, { "Turion(tm) 64",          4 }, "Turion 64 (1024K)",              UNKN_STR },
	{ 15, -1, -1, 15,   -1,   2,   512,    -1, { "Turion(tm) X2",          4 }, "Turion 64 X2 (512K)",            UNKN_STR },
	{ 15, -1, -1, 15,   -1,   2,  1024,    -1, { "Turion(tm) X2",          4 }, "Turion 64 X2 (1024K)",           UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,   128,    -1, { "Sempron(tm)",            2 }, "A64 Sempron (128K)",             UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,   256,    -1, { "Sempron(tm)",            2 }, "A64 Sempron (256K)",             UNKN_STR },
	{ 15, -1, -1, 15,   -1,   1,   512,    -1, { "Sempron(tm)",            2 }, "A64 Sempron (512K)",             UNKN_STR },
	{ 15, -1, -1, 15, 0x4f,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Orleans/512K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x5f,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Orleans/512K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x2f,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Venice/512K)",        "90 nm"  },
	{ 15, -1, -1, 15, 0x2c,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Venice/512K)",        "90 nm"  },
	{ 15, -1, -1, 15, 0x1f,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Winchester/512K)",    "90 nm"  },
	{ 15, -1, -1, 15, 0x0c,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Newcastle/512K)",     "130 nm" },
	{ 15, -1, -1, 15, 0x27,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (San Diego/512K)",     "90 nm"  },
	{ 15, -1, -1, 15, 0x37,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (San Diego/512K)",     "90 nm"  },
	{ 15, -1, -1, 15, 0x04,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (ClawHammer/512K)",    "130 nm" },

	{ 15, -1, -1, 15, 0x5f,   1,  1024,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Orleans/1024K)",      "90 nm"  },
	{ 15, -1, -1, 15, 0x27,   1,  1024,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (San Diego/1024K)",    "90 nm"  },
	{ 15, -1, -1, 15, 0x04,   1,  1024,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (ClawHammer/1024K)",   "130 nm" },

	{ 15, -1, -1, 15, 0x4b,   2,   256,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Windsor/256K)",    "90 nm" },

	{ 15, -1, -1, 15, 0x23,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Toledo/512K)",     "90 nm" },
	{ 15, -1, -1, 15, 0x4b,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Windsor/512K)",    "90 nm" },
	{ 15, -1, -1, 15, 0x43,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Windsor/512K)",    "90 nm" },
	{ 15, -1, -1, 15, 0x6b,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Brisbane/512K)",   "65 nm" },
	{ 15, -1, -1, 15, 0x2b,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Manchester/512K)", "90 nm" },

	{ 15, -1, -1, 15, 0x23,   2,  1024,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Toledo/1024K)",    "90 nm" },
	{ 15, -1, -1, 15, 0x43,   2,  1024,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon 64 X2 (Windsor/1024K)",   "90 nm" },

	{ 15, -1, -1, 15, 0x08,   1,   128,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Dublin/128K)", "130 nm" },
	{ 15, -1, -1, 15, 0x08,   1,   256,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Dublin/256K)", "130 nm" },
	{ 15, -1, -1, 15, 0x0c,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Paris)",              "130 nm" },
	{ 15, -1, -1, 15, 0x1c,   1,   128,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Palermo/128K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x1c,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Palermo/256K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x1c,   1,   128,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Sonora/128K)", "90 nm"  },
	{ 15, -1, -1, 15, 0x1c,   1,   256,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Sonora/256K)", "90 nm"  },
	{ 15, -1, -1, 15, 0x2c,   1,   128,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Palermo/128K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x2c,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Palermo/256K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x2c,   1,   128,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Albany/128K)", "90 nm"  },
	{ 15, -1, -1, 15, 0x2c,   1,   256,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Albany/256K)", "90 nm"  },
	{ 15, -1, -1, 15, 0x2f,   1,   128,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Palermo/128K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x2f,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Palermo/256K)",       "90 nm"  },
	{ 15, -1, -1, 15, 0x4f,   1,   128,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Manila/128K)",        "90 nm"  },
	{ 15, -1, -1, 15, 0x4f,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Manila/256K)",        "90 nm"  },
	{ 15, -1, -1, 15, 0x5f,   1,   128,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Manila/128K)",        "90 nm"  },
	{ 15, -1, -1, 15, 0x5f,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Manila/256K)",        "90 nm"  },
	{ 15, -1, -1, 15, 0x6b,   2,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 Dual (Sherman/256K)",  "65 nm"  },
	{ 15, -1, -1, 15, 0x6b,   2,   512,    -1, { "Sempron(tm)",            2 }, "Sempron 64 Dual (Sherman/512K)",  "65 nm"  },
	{ 15, -1, -1, 15, 0x7c,   1,   512,    -1, { "Athlon(tm) 64",          4 }, "Athlon 64 (Sherman/512K)",        "65 nm"  },
	{ 15, -1, -1, 15, 0x7f,   1,   256,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Sparta/256K)",        "65 nm"  },
	{ 15, -1, -1, 15, 0x7f,   1,   512,    -1, { "Sempron(tm)",            2 }, "Sempron 64 (Sparta/512K)",        "65 nm"  },
	{ 15, -1, -1, 15, 0x4c,   1,   256,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Keene/256K)",  "90 nm"  },
	{ 15, -1, -1, 15, 0x4c,   1,   512,    -1, { "Mobile AMD Sempron(tm)", 6 }, "Mobile Sempron 64 (Keene/512K)",  "90 nm"  },

	{ 15, -1, -1, 15, 0x24,   1,   512,    -1, { "Turion(tm) 64",          4 }, "Turion 64 (Lancaster/512K)",      "90 nm" },
	{ 15, -1, -1, 15, 0x24,   1,  1024,    -1, { "Turion(tm) 64",          4 }, "Turion 64 (Lancaster/1024K)",     "90 nm" },
	{ 15, -1, -1, 15, 0x48,   2,   256,    -1, { "Turion(tm) X2",          4 }, "Turion X2 (Taylor)",              "90 nm" },
	{ 15, -1, -1, 15, 0x48,   2,   512,    -1, { "Turion(tm) X2",          4 }, "Turion X2 (Trinidad)",            "90 nm" },
	{ 15, -1, -1, 15, 0x4c,   1,   512,    -1, { "Turion(tm) 64",          4 }, "Turion 64 (Richmond)",            "90 nm" },
	{ 15, -1, -1, 15, 0x68,   2,   256,    -1, { "Turion(tm) X2",          4 }, "Turion X2 (Tyler/256K)",          "65 nm" },
	{ 15, -1, -1, 15, 0x68,   2,   512,    -1, { "Turion(tm) X2",          4 }, "Turion X2 (Tyler/512K)",          "65 nm" },
	{ 15, -1, -1, 17,    3,   2,   512,    -1, { "Turion(tm) X2",          4 }, "Turion X2 (Griffin/512K)",        "65 nm" },
	{ 15, -1, -1, 17,    3,   2,  1024,    -1, { "Turion(tm) X2",          4 }, "Turion X2 (Griffin/1024K)",       "65 nm" },

	/* K10 Architecture (2007) */
	{ 15,  2, -1, 16,   -1,   3,    -1,    -1, { "Phenom(tm)",             2 }, "Phenom X3 (Toliman)",            "65 nm" },
	{ 15,  2, -1, 16,   -1,   4,    -1,    -1, { "Phenom(tm)",             2 }, "Phenom X4 (Agena)",              "65 nm" },
	{ 15,  2, -1, 16,   -1,   3,   512,    -1, { "Phenom(tm)",             2 }, "Phenom X3 (Toliman/256K)",       "65 nm" },
	{ 15,  2, -1, 16,   -1,   3,   512,    -1, { "Phenom(tm)",             2 }, "Phenom X3 (Toliman/512K)",       "65 nm" },
	{ 15,  2, -1, 16,   -1,   4,   128,    -1, { "Phenom(tm)",             2 }, "Phenom X4 (Agena/128K)",         "65 nm" },
	{ 15,  2, -1, 16,   -1,   4,   256,    -1, { "Phenom(tm)",             2 }, "Phenom X4 (Agena/256K)",         "65 nm" },
	{ 15,  2, -1, 16,   -1,   4,   512,    -1, { "Phenom(tm)",             2 }, "Phenom X4 (Agena/512K)",         "65 nm" },
	{ 15,  2, -1, 16,   -1,   2,   512,    -1, { "Athlon(tm) 64 X2",       6 }, "Athlon X2 (Kuma)",               "65 nm" },
	/* Phenom II derivates: */
	{ 15,  4, -1, 16,   -1,   1,  1024,    -1, { "Sempron(tm)",            2 }, "Sempron (Sargas)",               "45 nm" },
	{ 15,  4, -1, 16,   -1,   2,   512,    -1, { "Phenom(tm) II",          4 }, "Phenom II X2 (Callisto)",        "45 nm" },
	{ 15,  4, -1, 16,   -1,   3,   512,    -1, { "Phenom(tm) II",          4 }, "Phenom II X3 (Heka)",            "45 nm" },
	{ 15,  4, -1, 16,    4,   4,   512,    -1, { "Phenom(tm) II",          4 }, "Phenom II X4 (Deneb)",           "45 nm" },
	{ 15,  5, -1, 16,    5,   4,   512,    -1, { "Phenom(tm) II",          4 }, "Phenom II X4 (Deneb)",           "45 nm" },
	{ 15,  4, -1, 16,   10,   4,   512,    -1, { "Phenom(tm) II",          4 }, "Phenom II X4 (Zosma)",           "45 nm" },
	{ 15,  4, -1, 16,   10,   6,   512,    -1, { "Phenom(tm) II",          4 }, "Phenom II X6 (Thuban)",          "45 nm" },
	/* Athlon II derivates: */
	{ 15,  6, -1, 16,    6,   2,   512,    -1, { "Athlon(tm) II",          4 }, "Athlon II (Champlain)",          "45 nm" },
	{ 15,  6, -1, 16,    6,   2,   512,    -1, { "Athlon(tm) II X2",       6 }, "Athlon II X2 (Regor)",           "45 nm" },
	{ 15,  6, -1, 16,    6,   2,  1024,    -1, { "Athlon(tm) II X2",       6 }, "Athlon II X2 (Regor)",           "45 nm" },
	{ 15,  5, -1, 16,    5,   3,   512,    -1, { "Athlon(tm) II X3",       6 }, "Athlon II X3 (Rana)",            "45 nm" },
	{ 15,  5, -1, 16,    5,   4,   512,    -1, { "Athlon(tm) X4",          4 }, "Athlon II X4 (Propus)",          "45 nm" },
	{ 15,  5, -1, 16,    5,   4,   512,    -1, { "Athlon(tm) II X4",       6 }, "Athlon II X4 (Propus)",          "45 nm" },
	/* Opteron derivates: */
	{ 15,  2, -1, 16,    2,  -1,    -1,    -1, { "Opteron(tm) [28]3##",    4 }, "Opteron (Barcelona)",   "45 nm" },
	{ 15,  4, -1, 16,    4,  -1,    -1,    -1, { "Opteron(tm) [28]3##",    4 }, "Opteron (Shanghai)",    "45 nm" },
	{ 15,  8, -1, 16,    8,  -1,    -1,    -1, { "Opteron(tm) [28]4##",    4 }, "Opteron (Istanbul)",    "45 nm" },
	{ 15,  8, -1, 16,    8,  -1,    -1,    -1, { "Opteron(tm) 41##",       4 }, "Opteron (Lisbon)",      "45 nm" },
	{ 15,  9, -1, 16,    9,   8,    -1,    -1, { "Opteron(tm) 64##",       4 }, "Opteron (Magny-Cours)", "45 nm" },

	/* Llano APUs (2011): */
	{ 15,  1, -1, 18,    1,   4,    -1,    -1, { "Athlon(tm) II X4 6##",     6 }, "Athlon II X4 (Llano)", "GF 32SHP" },
	{ 15,  1, -1, 18,    1,   2,    -1,    -1, { "Athlon(tm) II X2 2##",     6 }, "Athlon II X2 (Llano)", "GF 32SHP" },
	{ 15,  1, -1, 18,    1,   2,    -1,    -1, { "Sempron(tm) X2 1##",       6 }, "Sempron X2 (Llano)",   "GF 32SHP" },
	{ 15,  1, -1, 18,    1,  -1,    -1,    -1, { "E2-3###",                  4 }, "E-Series (Llano)",     "GF 32SHP" },
	{ 15,  1, -1, 18,    1,  -1,    -1,    -1, { "A[468]-3###",              4 }, "A-Series (Llano)",     "GF 32SHP" },

	/* Family 14h: Bobcat Architecture (2011) */
	{ 15,  1, -1, 20,   -1,  -1,    -1,    -1, { "C-[356]#",                 4 }, "C-Series (Ontario)",  "TSMC N40" },
	{ 15,  1, -1, 20,   -1,  -1,    -1,    -1, { "E-[234]##",                4 }, "E-Series (Zacate)",   "TSMC N40" },
	{ 15,  1, -1, 20,   -1,  -1,    -1,    -1, { "G-T##[LRNE]",              6 }, "G-Series (Zacate)",   "TSMC N40" },
	{ 15,  1, -1, 20,   -1,  -1,    -1,    -1, { "Z-##",                     4 }, "Z-Series (Desna)",    "TSMC N40" },

	/* Family 15h: Bulldozer Architecture (2011) */
	{ 15, -1, -1, 21,    0,  -1,    -1,    -1, { "FX(tm)-[468]###",          4 }, "FX (Zambezi)",         "GF 32SHP" },
	{ 15, -1, -1, 21,    1,  -1,    -1,    -1, { "FX(tm)-[468]###",          4 }, "FX (Zambezi)",         "GF 32SHP" },
	{ 15, -1, -1, 21,    1,  -1,    -1,    -1, { "Opteron(tm)",              2 }, "Opteron (Interlagos)", "GF 32SHP" },
	/* 2nd-gen, Piledriver core (2012): */
	{ 15, -1, -1, 21,    2,  -1,    -1,    -1, { "FX(tm)-[4689]###",         4 }, "FX (Vishera)",          "GF 32SHP" },
	{ 15,  0, -1, 21,   16,   4,    -1,    -1, { "Athlon(tm) X4 7##",        6 }, "Athlon X4 (Trinity)",   "GF 32SHP" },
	{ 15,  0, -1, 21,   16,   2,    -1,    -1, { "Athlon(tm) X2 3##",        6 }, "Athlon X2 (Trinity)",   "GF 32SHP" },
	{ 15,  0, -1, 21,   16,   2,    -1,    -1, { "Sempron(tm) X2 2##",       6 }, "Sempron X2 (Trinity)",  "GF 32SHP" },
	{ 15,  0, -1, 21,   16,  -1,    -1,    -1, { "A[468]-5###",              4 }, "A-Series (Trinity)",    "GF 32SHP" },
	{ 15,  0, -1, 21,   16,  -1,    -1,    -1, { "A10-5###",                 4 }, "A-Series (Trinity)",    "GF 32SHP" },
	{ 15,  0, -1, 21,   16,  -1,    -1,    -1, { "A[468]-4###M",             6 }, "A-Series (Trinity)",    "GF 32SHP" },
	{ 15,  0, -1, 21,   16,  -1,    -1,    -1, { "A10-4###M",                6 }, "A-Series (Trinity)",    "GF 32SHP" },
	{ 15,  0, -1, 21,   19,  -1,    -1,    -1, { "FX(tm)-6##K",              6 }, "FX (Richland)",         "GF 32SHP" },
	{ 15,  0, -1, 21,   19,   4,    -1,    -1, { "Athlon(tm) X4 7##",        6 }, "Athlon X4 (Richland)",  "GF 32SHP" },
	{ 15,  0, -1, 21,   19,   2,    -1,    -1, { "Athlon(tm) X2 3##",        6 }, "Athlon X2 (Richland)",  "GF 32SHP" },
	{ 15,  0, -1, 21,   19,   2,    -1,    -1, { "Sempron(tm) X2 2##",       6 }, "Sempron X2 (Richland)", "GF 32SHP" },
	{ 15,  0, -1, 21,   19,  -1,    -1,    -1, { "A4 PRO-7###B",             8 }, "A-Series (Richland)",   "GF 32SHP" },
	{ 15,  0, -1, 21,   19,  -1,    -1,    -1, { "A[468]-[467]###",          4 }, "A-Series (Richland)",   "GF 32SHP" },
	{ 15,  0, -1, 21,   19,  -1,    -1,    -1, { "A10-6###",                 4 }, "A-Series (Richland)",   "GF 32SHP" },
	{ 15,  0, -1, 21,   19,  -1,    -1,    -1, { "A[468]-5###M",             6 }, "A-Series (Richland)",   "GF 32SHP" },
	{ 15,  0, -1, 21,   19,  -1,    -1,    -1, { "A10-5###M",                6 }, "A-Series (Richland)",   "GF 32SHP" },
	{ 15,  2, -1, 21,    2,  -1,    -1,    -1, { "Opteron(tm)",              2 }, "Opteron (Abu Dhabi)",   "GF 32SHP" },
	/* 3rd-gen, Steamroller core (2014): */
	{ 15,  8, -1, 21,   48,  -1,    -1,    -1, { "FX(tm)-7##K",              6 }, "FX (Kaveri)",            "TSMC N28" },
	{ 15,  8, -1, 21,   48,  -1,    -1,    -1, { "FX(tm)-7###",              4 }, "FX (Kaveri)",            "TSMC N28" },
	{ 15,  8, -1, 21,   48,   4,    -1,    -1, { "Athlon(tm) X4 8##",        6 }, "Athlon X4 (Kaveri)",     "TSMC N28" },
	{ 15,  8, -1, 21,   48,   2,    -1,    -1, { "Athlon(tm) X2 4##",        6 }, "Athlon X2 (Kaveri)",     "TSMC N28" },
	{ 15,  8, -1, 21,   48,  -1,    -1,    -1, { "A[468] PRO-[78]###B",      8 }, "A-Series (Kaveri)",      "TSMC N28" },
	{ 15,  8, -1, 21,   48,  -1,    -1,    -1, { "A[468]-7###",              4 }, "A-Series (Kaveri)",      "TSMC N28" },
	{ 15,  8, -1, 21,   48,  -1,    -1,    -1, { "A10 PRO-[78]###B",         8 }, "A-Series (Kaveri)",      "TSMC N28" },
	{ 15,  8, -1, 21,   48,  -1,    -1,    -1, { "A10-7###",                 4 }, "A-Series (Kaveri)",      "TSMC N28" },
	{ 15,  8, -1, 21,   56,  -1,    -1,    -1, { "A[468]-[78]###",           4 }, "A-Series (Godavari)",    "TSMC N28" },
	{ 15,  8, -1, 21,   56,  -1,    -1,    -1, { "A10-[78]###",              4 }, "A-Series (Godavari)",    "TSMC N28" },
	{ 15,  8, -1, 21,   56,   4,    -1,    -1, { "Athlon(tm) X4 8##",        6 }, "Athlon X4 (Godavari)",   "TSMC N28" },
	{ 15,  0, -1, 21,   48,  -1,    -1,    -1, { "RX-###",                   4 }, "R-Series (Bald Eagle)",  "TSMC N28" },
	/* 4th-gen, Excavator core (2015): */
	{ 15,  0, -1, 21,   96,  -1,    -1,    -1, { "FX-8###P",                 6 }, "FX (Carrizo)",              "GF 28SHP" },
	{ 15,  0, -1, 21,   96,   4,    -1,    -1, { "Athlon(tm) X4 8##",        6 }, "Athlon X4 (Carrizo)",       "GF 28SHP" },
	{ 15,  0, -1, 21,   96,  -1,    -1,    -1, { "A[68] PRO-8###",           6 }, "A-Series (Carrizo)",        "GF 28SHP" },
	{ 15,  0, -1, 21,   96,  -1,    -1,    -1, { "A[68]-[78]###",            4 }, "A-Series (Carrizo)",        "GF 28SHP" },
	{ 15,  0, -1, 21,   96,  -1,    -1,    -1, { "A1[02] PRO-8###",          6 }, "A-Series (Carrizo)",        "GF 28SHP" },
	{ 15,  0, -1, 21,   96,  -1,    -1,    -1, { "A1[02]-8###",              4 }, "A-Series (Carrizo)",        "GF 28SHP" },
	{ 15,  5, -1, 21,  101,  -1,    -1,    -1, { "FX-9###P",                 6 }, "FX (Bristol Ridge)",        "GF 28SHP" },
	{ 15,  5, -1, 21,  101,  -1,    -1,    -1, { "Athlon(tm) X4 9##",        6 }, "Athlon X4 (Bristol Ridge)", "GF 28SHP" },
	{ 15,  5, -1, 21,  101,  -1,    -1,    -1, { "A[68] PRO-9###",           6 }, "A-Series (Bristol Ridge)",  "GF 28SHP" },
	{ 15,  5, -1, 21,  101,  -1,    -1,    -1, { "A[68]-9###",               4 }, "A-Series (Bristol Ridge)",  "GF 28SHP" },
	{ 15,  5, -1, 21,  101,  -1,    -1,    -1, { "A1[02] PRO-9###",          6 }, "A-Series (Bristol Ridge)",  "GF 28SHP" },
	{ 15,  5, -1, 21,  101,  -1,    -1,    -1, { "A1[02]-9###",              4 }, "A-Series (Bristol Ridge)",  "GF 28SHP" },
	{ 15,  0, -1, 21,  112,   2,    -1,    -1, { "A[469]-9###",              4 }, "A-Series (Stoney Ridge)",   "GF 28SHP" },
	{ 15,  0, -1, 21,  112,  -1,    -1,    -1, { "E2-9###",                  4 }, "E-Series (Stoney Ridge)",   "GF 28SHP" },
	{ 15,  0, -1, 21,   96,  -1,    -1,    -1, { "Opteron(tm) X3###",        6 }, "Opteron (Toronto)",         "GF 28SHP" },

	/* Family 16h: Jaguar Architecture (2013) */
	{ 15,  0, -1, 22,    0,   4,    -1,    -1, { "Athlon(tm) X4 5##",        6 }, "Athlon X4 (Kabini)",  "TSMC N28" },
	{ 15,  0, -1, 22,    0,   4,    -1,    -1, { "Athlon(tm) 5###",          4 }, "Athlon X4 (Kabini)",  "TSMC N28" },
	{ 15,  0, -1, 22,    0,  -1,    -1,    -1, { "Sempron(tm) [23]###",      4 }, "Sempron (Kabini)",    "TSMC N28" },
	{ 15,  0, -1, 22,    0,  -1,    -1,    -1, { "E1-2###",                  4 }, "E-Series (Kabini)",   "TSMC N28" },
	{ 15,  0, -1, 22,    0,  -1,    -1,    -1, { "E2-3###",                  4 }, "E-Series (Kabini)",   "TSMC N28" },
	{ 15,  0, -1, 22,    0,  -1,    -1,    -1, { "A4 PRO-3###",              6 }, "A-Series (Kabini)",   "TSMC N28" },
	{ 15,  0, -1, 22,    0,  -1,    -1,    -1, { "A[46]-5###",               4 }, "A-Series (Kabini)",   "TSMC N28" },
	/* 2nd-gen, Puma core (2013): */
	{ 15,  0, -1, 22,   48,   2,    -1,    -1, { "E1 Micro-62##T",           8 }, "E-Series (Mullins)",       "GF 28SHP" },
	{ 15,  0, -1, 22,   48,   4,    -1,    -1, { "A4 Micro-64##T",           8 }, "A-Series (Mullins)",       "GF 28SHP" },
	{ 15,  0, -1, 22,   48,   4,    -1,    -1, { "A10 Micro-67##T",          8 }, "A-Series (Mullins)",       "GF 28SHP" },
	{ 15,  0,  1, 22,   48,  -1,    -1,    -1, { "E[12]-6###",               4 }, "E-Series (Beema)",         "GF 28SHP" },
	{ 15,  0,  1, 22,   48,  -1,    -1,    -1, { "A[468]-6###",              4 }, "A-Series (Beema)",         "GF 28SHP" },
	{ 15,  0,  1, 22,   48,  -1,    -1,    -1, { "GX-###",                   4 }, "G-Series (Steppe Eagle)",  "GF 28SHP" },

	/* Family 17h */
	/* Zen (2017) => https://en.wikichip.org/wiki/amd/microarchitectures/zen */
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "EPYC 7##1",              4 }, "EPYC (Naples)",                  "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Threadripper 1###",      4 }, "Threadripper (Whitehaven)",      "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Ryzen 7 PRO 1###",       8 }, "Ryzen 7 PRO (Summit Ridge)",     "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Ryzen 7 1###",           6 }, "Ryzen 7 (Summit Ridge)",         "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Ryzen 5 PRO 1###",       8 }, "Ryzen 5 PRO (Summit Ridge)",     "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Ryzen 5 1###",           6 }, "Ryzen 5 (Summit Ridge)",         "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Ryzen 3 PRO 1###",       8 }, "Ryzen 3 PRO (Summit Ridge)",     "GF 14LP" },
	{ 15, -1, -1, 23,    1,  -1,    -1,    -1, { "Ryzen 3 1###",           6 }, "Ryzen 3 (Summit Ridge)",         "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Ryzen PRO 7 2###",       8 }, "Ryzen 7 PRO (Raven Ridge)",      "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Ryzen 7 2###",           6 }, "Ryzen 7 (Raven Ridge)",          "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Ryzen PRO 5 2###",       8 }, "Ryzen 5 PRO (Raven Ridge)",      "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Ryzen 5 2###",           6 }, "Ryzen 5 (Raven Ridge)",          "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Ryzen PRO 3 2###",       8 }, "Ryzen 3 PRO (Raven Ridge)",      "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Ryzen 3 2###",           6 }, "Ryzen 3 (Raven Ridge)",          "GF 14LP" },
	{ 15, -1, -1, 23,   17,  -1,    -1,    -1, { "Athlon",                 2 }, "Athlon (Raven Ridge)",           "GF 14LP" },
	{ 15, -1, -1, 23,   32,  -1,    -1,    -1, { "Ryzen 3 3###",           6 }, "Ryzen 3 (Dali)",                 "GF 14LP" },
	{ 15, -1, -1, 23,   32,  -1,    -1,    -1, { "Athlon",                 2 }, "Athlon (Dali)",                  "GF 14LP" },
	{ 15, -1,  1, 23,   32,  -1,    -1,    -1, { "",                       0 }, "Dali",                           "GF 14LP" }, /* AMD 3020e */
	/* Zen+ (2018) => https://en.wikichip.org/wiki/amd/microarchitectures/zen%2B */
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Threadripper 2###",      4 }, "Threadripper (Colfax)",          "GF 12LP" },
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Ryzen 7 PRO 2###",       8 }, "Ryzen 7 PRO (Pinnacle Ridge)",   "GF 12LP" },
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Ryzen 7 2###",           6 }, "Ryzen 7 (Pinnacle Ridge)",       "GF 12LP" },
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Ryzen 5 PRO 2###",       8 }, "Ryzen 5 PRO (Pinnacle Ridge)",   "GF 12LP" },
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Ryzen 5 2###",           6 }, "Ryzen 5 (Pinnacle Ridge)",       "GF 12LP" },
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Ryzen 3 PRO 2###",       8 }, "Ryzen 3 PRO (Pinnacle Ridge)",   "GF 12LP" },
	{ 15, -1, -1, 23,    8,  -1,    -1,    -1, { "Ryzen 3 2###",           6 }, "Ryzen 3 (Pinnacle Ridge)",       "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Ryzen 7 PRO 3###",       8 }, "Ryzen 7 PRO (Picasso)",          "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Ryzen 7 3###",           6 }, "Ryzen 7 (Picasso)",              "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Ryzen 5 PRO 3###",       8 }, "Ryzen 5 PRO (Picasso)",          "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Ryzen 5 3###",           6 }, "Ryzen 5 (Picasso)",              "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Ryzen 3 PRO 3###",       8 }, "Ryzen 3 PRO (Picasso)",          "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Ryzen 3 3###",           6 }, "Ryzen 3 (Picasso)",              "GF 12LP" },
	{ 15, -1, -1, 23,   24,  -1,    -1,    -1, { "Athlon",                 2 }, "Athlon (Picasso)",               "GF 12LP" },
	/* Zen 2 (2019) => https://en.wikichip.org/wiki/amd/microarchitectures/zen_2 */
	{ 15, -1, -1, 23,   49,  -1,    -1,    -1, { "EPYC 7##2",                4 }, "EPYC (Rome)",                    "TSMC N7FF" },
	{ 15, -1, -1, 23,   49,  -1,    -1,    -1, { "Threadripper PRO 3###WX", 10 }, "Threadripper PRO (Castle Peak)", "TSMC N7FF" },
	{ 15, -1, -1, 23,   49,  -1,    -1,    -1, { "Threadripper 3###X",       6 }, "Threadripper (Castle Peak)",     "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 9 PRO 3###",       8 }, "Ryzen 9 PRO (Matisse)",          "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 9 3###",           6 }, "Ryzen 9 (Matisse)",              "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 7 PRO 3###",       8 }, "Ryzen 7 PRO (Matisse)",          "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 7 3###",           6 }, "Ryzen 7 (Matisse)",              "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 5 PRO 3###",       8 }, "Ryzen 5 PRO (Matisse)",          "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 5 3###",           6 }, "Ryzen 5 (Matisse)",              "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 3 PRO 3###",       8 }, "Ryzen 3 PRO (Matisse)",          "TSMC N7FF" },
	{ 15, -1, -1, 23,  113,  -1,    -1,    -1, { "Ryzen 3 3###",           6 }, "Ryzen 3 (Matisse)",              "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 9 PRO 4###",       8 }, "Ryzen 9 PRO (Renoir)",           "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 9 4###",           6 }, "Ryzen 9 (Renoir)",               "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 7 PRO 4###",       8 }, "Ryzen 7 PRO (Renoir)",           "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 7 4###",           6 }, "Ryzen 7 (Renoir)",               "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 5 PRO 4###",       8 }, "Ryzen 5 PRO (Renoir)",           "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 5 4###",           6 }, "Ryzen 5 (Renoir)",               "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 3 PRO 4###",       8 }, "Ryzen 3 PRO (Renoir)",           "TSMC N7FF" },
	{ 15, -1, -1, 23,   96,  -1,    -1,    -1, { "Ryzen 3 4###",           6 }, "Ryzen 3 (Renoir)",               "TSMC N7FF" },
	{ 15, -1, -1, 23,  104,  -1,    -1,    -1, { "Ryzen 7 5###",           6 }, "Ryzen 7 (Lucienne)",             "TSMC N7FF" },
	{ 15, -1, -1, 23,  104,  -1,    -1,    -1, { "Ryzen 5 5###",           6 }, "Ryzen 5 (Lucienne)",             "TSMC N7FF" },
	{ 15, -1, -1, 23,  104,  -1,    -1,    -1, { "Ryzen 3 5###",           6 }, "Ryzen 3 (Lucienne)",             "TSMC N7FF" },
	{ 15, -1, -1, 23,   71,  -1,    -1,    -1, { "Desktop Kit",            4 }, "Desktop Kit (Zen 2)",            "TSMC N7FF" }, /* 4700S Desktop Kit */
	{ 15, -1, -1, 23,  132,  -1,    -1,    -1, { "Desktop Kit",            4 }, "Desktop Kit (Zen 2)",            "TSMC N7FF" }, /* 4800S Desktop Kit */
	{ 15, -1,  2, 23,  144,  -1,    -1,    -1, { "Custom APU",             4 }, "Van Gogh",                       "TSMC N7FF" }, /* Custom APU 0405 */
	{ 15, -1,  0, 23,  145,  -1,    -1,    -1, { "Custom APU",             4 }, "Van Gogh",                       "TSMC N7FF" }, /* Custom APU 0932 */
	{ 15, -1, -1, 23,  160,  -1,    -1,    -1, { "Ryzen 5 7###",           6 }, "Ryzen 5 (Mendocino)",            "TSMC N6"   },
	{ 15, -1, -1, 23,  160,  -1,    -1,    -1, { "Ryzen 3 7###",           6 }, "Ryzen 3 (Mendocino)",            "TSMC N6"   },
	{ 15, -1, -1, 23,  160,  -1,    -1,    -1, { "Athlon",                 2 }, "Athlon (Mendocino)",             "TSMC N6"   },

	/* Family 18h */
	/* Zen Architecture for Hygon (2018) => https://en.wikichip.org/wiki/hygon/microarchitectures/dhyana */
	{ 15, -1, -1, 24,    0,  -1,    -1,    -1, { "C86",                    2 }, "C86 (Dhyana)",                   UNKN_STR },

	/* Family 19h */
	/* Zen 3 (2020) => https://en.wikichip.org/wiki/amd/microarchitectures/zen_3 */
	{ 15, -1, -1, 25,    1,  -1,    -1,    -1, { "EPYC 7##3",                4 }, "EPYC (Milan)",                 "TSMC N7FF" },
	{ 15, -1, -1, 25,    8,  -1,    -1,    -1, { "Threadripper PRO 5###WX", 10 }, "Threadripper PRO (Chagall)",   "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 9 PRO 5###",       8 }, "Ryzen 9 PRO (Vermeer)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 9 5###",           6 }, "Ryzen 9 (Vermeer)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 7 PRO 5###",       8 }, "Ryzen 7 PRO (Vermeer)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 7 5###",           6 }, "Ryzen 7 (Vermeer)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 5 PRO 5###",       8 }, "Ryzen 5 PRO (Vermeer)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 5 5###",           6 }, "Ryzen 5 (Vermeer)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 3 PRO 5###",       8 }, "Ryzen 3 PRO (Vermeer)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   33,  -1,    -1,    -1, { "Ryzen 3 5###",           6 }, "Ryzen 3 (Vermeer)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 9 PRO 5##0[HU]",  10 }, "Ryzen 9 PRO (Cezanne)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 9 5##0[HU]",       8 }, "Ryzen 9 (Cezanne)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 7 PRO 5##0[HU]",  10 }, "Ryzen 7 PRO (Cezanne)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 7 5##0[HU]",       8 }, "Ryzen 7 (Cezanne)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 5 PRO 5##0[HU]",  10 }, "Ryzen 5 PRO (Cezanne)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 5 5##0[HU]",       8 }, "Ryzen 5 (Cezanne)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 3 PRO 5##0[HU]",  10 }, "Ryzen 3 PRO (Cezanne)",          "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 3 5##0[HU]",       8 }, "Ryzen 3 (Cezanne)",              "TSMC N7FF" },
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 7 5##5U",          8 }, "Ryzen 7 (Barceló)",              "TSMC N7FF" }, /* Ryzen 7 5825U */
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 5 5##5U",          8 }, "Ryzen 5 (Barceló)",              "TSMC N7FF" }, /* Ryzen 5 5625U */
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 3 PRO 5##5U",     10 }, "Ryzen 3 PRO (Barceló)",          "TSMC N7FF" }, /* Ryzen 3 PRO 5475U */
	{ 15, -1, -1, 25,   80,  -1,    -1,    -1, { "Ryzen 3 5##5C",          8 }, "Ryzen 3 (Barceló)",              "TSMC N7FF" }, /* Ryzen 3 5125C */
	/* Zen 3+ (2022) */
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 9 PRO 6###",       8 }, "Ryzen 9 PRO (Rembrandt)",        "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 9 6###",           6 }, "Ryzen 9 (Rembrandt)",            "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 7 PRO 6###",       8 }, "Ryzen 7 PRO (Rembrandt)",        "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 7 6###",           6 }, "Ryzen 7 (Rembrandt)",            "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 5 PRO 6###",       8 }, "Ryzen 5 PRO (Rembrandt)",        "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 5 6###",           6 }, "Ryzen 5 (Rembrandt)",            "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 3 PRO 6###",       8 }, "Ryzen 3 PRO (Rembrandt)",        "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 3 6###",           6 }, "Ryzen 3 (Rembrandt)",            "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 7 7###",           6 }, "Ryzen 7 (Rembrandt-R)",          "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 5 7###",           6 }, "Ryzen 5 (Rembrandt-R)",          "TSMC N6"  },
	{ 15, -1, -1, 25,   68,  -1,    -1,    -1, { "Ryzen 3 7###",           6 }, "Ryzen 3 (Rembrandt-R)",          "TSMC N6"  },
	/* Zen 4 (2022) => https://en.wikichip.org/wiki/amd/microarchitectures/zen_4 */
	{ 15, -1, -1, 25,   17,  -1,    -1,    -1, { "EPYC 9##4",                4 }, "EPYC (Genoa)",                   "TSMC N5"  },
	{ 15, -1, -1, 25,   24,  -1,    -1,    -1, { "Threadripper PRO 7###WX", 10 }, "Threadripper PRO (Storm Peak)",  "TSMC N5"  },
	{ 15, -1, -1, 25,   24,  -1,    -1,    -1, { "Threadripper 7###X",       6 }, "Threadripper (Storm Peak)",      "TSMC N5"  },
	/*  => Raphael (7000 series, Zen 4/RDNA2 based) */
	{ 15, -1,  2, 25,   97,  -1,    -1,    -1, { "Ryzen 9 7###",           6 }, "Ryzen 9 (Raphael)",              "TSMC N5"  },
	{ 15, -1,  2, 25,   97,  -1,    -1,    -1, { "Ryzen 7 7###",           6 }, "Ryzen 7 (Raphael)",              "TSMC N5"  },
	{ 15, -1,  2, 25,   97,  -1,    -1,    -1, { "Ryzen 5 7###",           6 }, "Ryzen 5 (Raphael)",              "TSMC N5"  },
	{ 15, -1,  2, 25,   97,  -1,    -1,    -1, { "Ryzen 3 7###",           6 }, "Ryzen 3 (Raphael)",              "TSMC N5"  },
	/*  => Dragon Range (7045 series, Zen 4/RDNA2 based) */
	{ 15, -1, -1, 25,   97,  -1,    -1,    -1, { "Ryzen 9 7###H",          8 }, "Ryzen 9 (Dragon Range)",         "TSMC N5"  },
	{ 15, -1, -1, 25,   97,  -1,    -1,    -1, { "Ryzen 7 7###H",          8 }, "Ryzen 7 (Dragon Range)",         "TSMC N5"  },
	{ 15, -1, -1, 25,   97,  -1,    -1,    -1, { "Ryzen 5 7###H",          8 }, "Ryzen 5 (Dragon Range)",         "TSMC N5"  },
	/*  => Phoenix (7040 series, Zen 4/RDNA3/XDNA based) */
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 9 PRO 7###[HU]",  10 }, "Ryzen 9 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 9 7###[HU]",       8 }, "Ryzen 9 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 7 PRO 7###[HU]",  10 }, "Ryzen 7 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 7 7###[HU]",       8 }, "Ryzen 7 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 5 PRO 7###[HU]",  10 }, "Ryzen 5 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 5 7###[HU]",       8 }, "Ryzen 5 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 3 PRO 7###[HU]",  10 }, "Ryzen 3 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen 3 7###[HU]",       8 }, "Ryzen 3 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  116,  -1,    -1,    -1, { "Ryzen Z1",               4 }, "Ryzen Z1 (Phoenix)",             "TSMC N4"  },
	/*  => Phoenix (8000 series, Zen 4 based) */
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 7 8###F",          8 }, "Ryzen 7 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 5 8###F",          8 }, "Ryzen 5 (Phoenix)",              "TSMC N4"  },
	/*  => Phoenix (8000 series with Radeon Graphics, Zen 4/RDNA3/XDNA based) */
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 9 PRO 8###G",     10 }, "Ryzen 9 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 9 8###G",          8 }, "Ryzen 9 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 7 PRO 8###G",     10 }, "Ryzen 7 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 7 8###G",          8 }, "Ryzen 7 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 5 PRO 8###G",     10 }, "Ryzen 5 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 5 8###G",          8 }, "Ryzen 5 (Phoenix)",              "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 3 PRO 8###G",     10 }, "Ryzen 3 PRO (Phoenix)",          "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 3 8###G",          8 }, "Ryzen 3 (Phoenix)",              "TSMC N4"  },
	/*  => Hawk Point (8040 series, Zen 4/RDNA3/XDNA based) */
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 9 PRO 8###[HU]",     10 }, "Ryzen 9 PRO (Hawk Point)",       "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 9 8###[HU]",          8 }, "Ryzen 9 (Hawk Point)",           "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 7 PRO 8###[HU]",     10 }, "Ryzen 7 PRO (Hawk Point)",       "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 7 8###[HU]",          8 }, "Ryzen 7 (Hawk Point)",           "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 5 PRO 8###[HU]",     10 }, "Ryzen 5 PRO (Hawk Point)",       "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 5 8###[HU]",          8 }, "Ryzen 5 (Hawk Point)",           "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 3 PRO 8###[HU]",     10 }, "Ryzen 3 PRO (Hawk Point)",       "TSMC N4"  },
	{ 15, -1, -1, 25,  117,  -1,    -1,    -1, { "Ryzen 3 8###[HU]",          8 }, "Ryzen 3 (Hawk Point)",           "TSMC N4"  },
	/* Zen 5 (2024) => https://en.wikichip.org/wiki/amd/microarchitectures/zen_5 */
	{ 15, -1, -1, 26,    2,  -1,    -1,    -1, { "EPYC 9##5",                4 }, "EPYC (Turin)",                    "TSMC N4X" },
	{ 15, -1, -1, 26,   17,  -1,    -1,    -1, { "EPYC 9##5",                4 }, "EPYC (Turin Dense)",              "TSMC N3E" },
	{ 15, -1, -1, 26,    8,  -1,    -1,    -1, { "Threadripper PRO 9###WX", 10 }, "Threadripper PRO (Shimada Peak)", "TSMC N4"  },
	{ 15, -1, -1, 26,    8,  -1,    -1,    -1, { "Threadripper 9###X",       6 }, "Threadripper (Shimada Peak)",     "TSMC N4"  },
	/*  => Granite Ridge (9000 series, Zen 5 based) */
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 9 PRO 9###",       8 }, "Ryzen 9 PRO (Granite Ridge)",    "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 9 9###",           6 }, "Ryzen 9 (Granite Ridge)",        "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 7 PRO 9###",       8 }, "Ryzen 7 PRO (Granite Ridge)",    "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 7 9###",           6 }, "Ryzen 7 (Granite Ridge)",        "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 5 PRO 9###",       8 }, "Ryzen 5 PRO (Granite Ridge)",    "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 5 9###",           6 }, "Ryzen 5 (Granite Ridge)",        "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 3 PRO 9###",       8 }, "Ryzen 3 PRO (Granite Ridge)",    "TSMC N4"  },
	{ 15, -1, -1, 26,   68,  -1,    -1,    -1, { "Ryzen 3 9###",           6 }, "Ryzen 3 (Granite Ridge)",        "TSMC N4"  },
	/*  => Strix Point and Krackan Point (Zen 5/RDNA3.5/XDNA2 based) */
	{ 15, -1, -1, 26,   36,  -1,    -1,    -1, { "Ryzen AI 9 HX PRO",     10 }, "Ryzen AI 9 PRO (Strix Point)",   "TSMC N4P" },
	{ 15, -1, -1, 26,   36,  -1,    -1,    -1, { "Ryzen AI 9",             6 }, "Ryzen AI 9 (Strix Point)",       "TSMC N4P" },
	{ 15, -1, -1, 26,   36,  -1,    -1,    -1, { "Ryzen AI 7 PRO",         8 }, "Ryzen AI 7 PRO (Strix Point)",   "TSMC N4P" }, /* Ryzen AI 7 PRO 360 */
	{ 15, -1, -1, 26,   96,  -1,    -1,    -1, { "Ryzen AI 7 PRO",         8 }, "Ryzen AI 7 PRO (Krackan Point)", "TSMC N4P" }, /* Ryzen AI 7 PRO 350 */
	{ 15, -1, -1, 26,   96,  -1,    -1,    -1, { "Ryzen AI 7",             6 }, "Ryzen AI 7 (Krackan Point)",     "TSMC N4P" }, /* Ryzen AI 7 350 */
	{ 15, -1, -1, 26,   96,  -1,    -1,    -1, { "Ryzen AI 5 PRO",         8 }, "Ryzen AI 5 PRO (Krackan Point)", "TSMC N4P" }, /* Ryzen AI 5 PRO 340 */
	{ 15, -1, -1, 26,   96,  -1,    -1,    -1, { "Ryzen AI 5",             6 }, "Ryzen AI 5 (Krackan Point)",     "TSMC N4P" }, /* Ryzen AI 5 340 */
	/* => Strix Halo (Zen 5/RDNA3.5/XDNA2 based) */
	{ 15, -1, -1, 26,  112,  -1,    -1,    -1, { "Ryzen AI MAX+ PRO",      10 }, "Ryzen AI MAX+ PRO (Strix Halo)", "TSMC N4P" }, /* Ryzen AI MAX+ PRO 395 */
	{ 15, -1, -1, 26,  112,  -1,    -1,    -1, { "Ryzen AI MAX+",          8 }, "Ryzen AI MAX+ (Strix Halo)",      "TSMC N4P" }, /* Ryzen AI MAX+ 395 */
	{ 15, -1, -1, 26,  112,  -1,    -1,    -1, { "Ryzen AI MAX PRO",       8 }, "Ryzen AI MAX PRO (Strix Halo)",   "TSMC N4P" },
	{ 15, -1, -1, 26,  112,  -1,    -1,    -1, { "Ryzen AI MAX",           6 }, "Ryzen AI MAX (Strix Halo)",       "TSMC N4P" },
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                          Codename                          Technology
};

/********************************************************************************** */

// https://github.com/anrieff/libcpuid/blob/ff6b7500351293259ca808783ee81e8ab5b7c0cb/libcpuid/recog_centaur.c
/*
 * Copyright 2023  Veselin Georgiev,
 * anrieffNOSPAM @ mgail_DOT.com (convert to gmail)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
const struct match_entry_t cpudb_centaur[] = {
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                              Codename                         Technology
	{ -1, -1, -1, -1,   -1,  -1,    -1,    -1, { "",                           0 }, "Unknown Centaur CPU",           ""       },
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                              Codename                         Technology


	/* VIA */
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                              Codename                         Technology
	{  6, -1, -1,  -1,   -1,  -1,    -1,    -1, { "VIA",                       2 }, "Unknown VIA CPU",               ""       },

	/* Samuel (2000, 180 nm) */
	{  6,   6, -1, -1,   -1,  -1,    -1,    -1, { "VIA Samuel",                4 }, "VIA Cyrix III (Samuel)",        "180 nm" },
	/* Samuel 2 (2001, 150 nm) */
	{  6,   7, -1, -1,   -1,  -1,    -1,    -1, { "VIA Samuel 2",              6 }, "VIA C3 (Samuel 2)",             "150 nm" },
	/* Ezra (2001, 130 nm) */
	{  6,   7, -1, -1,   -1,  -1,    -1,    -1, { "VIA Ezra",                  4 }, "VIA C3 (Ezra)",                 "130 nm" },
	{  6,   8, -1, -1,   -1,  -1,    -1,    -1, { "VIA C3 Ezra",               6 }, "VIA C3 (Ezra-T)",               "130 nm" },
	/* Nehemiah (2003, 130 nm) */
	{  6,   9, -1, -1,   -1,  -1,    -1,    -1, { "VIA Nehemiah",              4 }, "VIA C3 (Nehemiah)",             "130 nm" },
	/* Esther (2005, 90 nm) */
	{  6,  10, -1, -1,   -1,  -1,    -1,    -1, { "VIA Esther",                4 }, "VIA C7 (Esther)",               "90 nm"  },
	{  6,  13, -1, -1,   -1,  -1,    -1,    -1, { "VIA C7-M",                  4 }, "VIA C7-M (Esther)",             "90 nm"  },
	/* Isaiah (2008, 65 nm) */
	{  6,  15, -1, -1,   -1,  -1,    -1,    -1, { "VIA Nano",                  4 }, "VIA Nano (Isaiah)",             "65 nm"  },
	{  6,  15, -1, -1,   -1,   1,    -1,    -1, { "VIA Nano",                  4 }, "VIA Nano (Isaiah)",             "65 nm"  },
	{  6,  15, -1, -1,   -1,   2,    -1,    -1, { "VIA Nano",                  4 }, "VIA Nano X2 (Isaiah)",          "65 nm"  },
	{  6,  15, -1, -1,   -1,  -1,    -1,    -1, { "VIA QuadCore",              4 }, "VIA Nano X4 (Isaiah)",          "65 nm"  },
	{  6,  15, -1, -1,   -1,   4,    -1,    -1, { "VIA Eden X4",               6 }, "VIA Eden X4 (Isaiah)",          "65 nm"  },
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                              Codename                         Technology


	/* Zhaoxin */
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                              Codename                         Technology
	{  7, -1, -1, -1,   -1,  -1,    -1,    -1, {"ZHAOXIN",                     2 }, "Unknown Zhaoxin CPU",           ""       },

	/* Zhangjiang (2015, 28 nm) */
	{  7, -1, -1, -1,   15,  -1,    -1,    -1, { "ZHAOXIN KaisHeng KH-C",      8 }, "Zhaoxin KaisHeng (ZhangJiang)", "28 nm"  }, // C+ (4000)
	{  7, -1, -1, -1,   15,  -1,    -1,    -1, { "ZHAOXIN KaiXian ZX-C",       8 }, "Zhaoxin KaiXian (ZhangJiang)",  "28 nm"  }, // C/C+ (4000)
	/* WuDaoKou (2017, 28 nm) */
	{  7, -1, -1, -1,   27,  -1,    -1,    -1, { "ZHAOXIN KaisHeng KH-20###",  8 }, "Zhaoxin KaisHeng (WuDaoKou)",   "28 nm"  }, // KH (20000)
	{  7, -1, -1, -1,   27,  -1,    -1,    -1, { "ZHAOXIN KaiXian KX-5###",    8 }, "Zhaoxin KaiXian (WuDaoKou)",    "28 nm"  }, // KX (5000)
	{  7, -1, -1, -1,   27,  -1,    -1,    -1, { "ZHAOXIN KaiXian KX-U5###",   8 }, "Zhaoxin KaiXian (WuDaoKou)",    "28 nm"  }, // KX (U5000)
	/* LuJiaZui (2019, 16 nm) */
	{  7, -1, -1, -1,   59,  -1,    -1,    -1, { "ZHAOXIN KaisHeng KH-30###",  8 }, "Zhaoxin KaisHeng (LuJiaZui)",   "16 nm"  }, // KH (30000)
	{  7, -1, -1, -1,   59,  -1,    -1,    -1, { "ZHAOXIN KaiXian KX-6###",    8 }, "Zhaoxin KaiXian (LuJiaZui)",    "16 nm"  }, // KX (6000)
	{  7, -1, -1, -1,   59,  -1,    -1,    -1, { "ZHAOXIN KaiXian KX-U6###",   8 }, "Zhaoxin KaiXian (LuJiaZui)",    "16 nm"  }, // KX (U6000)
	/* Yongfeng (2022, 16 nm) */
	{  7, -1, -1, -1,   91,  -1,    -1,    -1, { "ZHAOXIN KaisHeng KH-40###",  8 }, "Zhaoxin KaisHeng (Yongfeng)",   "16 nm"  }, // KH (40000)
	{  7, -1, -1, -1,   91,  -1,    -1,    -1, { "ZHAOXIN KaiXian KX-7###",    8 }, "Zhaoxin KaiXian (Yongfeng)",    "16 nm"  }, // KX (7000)
//     F   M   S  EF    EM #cores  L2$    L3$  Pattern                              Codename                         Technology
};

// clang-format on

///////////////////////////////////////////////////////////////////////////////////////////////

static bool match_pattern_core(const char* p, const char* b) {
    while (*p && *b) {
        if (*p == '#') {
            // '#' digit
            if (!ffCharIsDigit(*b)) return false;
            p++; b++;
        } else if (*p == '[') {
            // '[' chars
            p++;
            bool found = false;
            while (*p && *p != ']') {
                if (*p == *b) found = true;
                p++;
            }
            if (!found) return false;
            if (*p == ']') p++;
            b++;
        } else {
            // char
            if (*p != *b) return false;
            p++; b++;
        }
    }

    // ignore suffix
    return (*p == '\0');
}

static bool match_brand_pattern(const char* pattern, const char* brand) {
    if (pattern[0] == '\0') return true;
    if (brand[0] == '\0') return false;

    for (const char* b = brand; *b != '\0'; b++) {
        if (match_pattern_core(pattern, b)) {
            return true;
        }
    }
    return false;
}

bool ffCPUDetectX86Specific(FFCPUResult* cpu) {
    // Ref: https://github.com/anrieff/libcpuid/blob/2e4456ae0165db3155da2e8fba92afd5c090ca1b/libcpuid/cpuid_main.c#L1096
    unsigned int eax, ebx, ecx, edx;

    // Vendor (CPUID Leaf 0)
    if (!__get_cpuid(0, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    if (!cpu->vendor.length) {
        ffStrbufEnsureFixedLengthFree(&cpu->vendor, 12);
        memcpy(cpu->vendor.chars + 0, &ebx, 4);
        memcpy(cpu->vendor.chars + 4, &edx, 4);
        memcpy(cpu->vendor.chars + 8, &ecx, 4);
        cpu->vendor.chars[12] = '\0';
        cpu->vendor.length = 12;
    }

    // CPU Version Info (CPUID Leaf 1)
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    int32_t family = (eax >> 8) & 0xF;
    int32_t model = (eax >> 4) & 0xF;
    int32_t stepping = eax & 0xF;
    int32_t xfamily = (eax >> 20) & 0xFF;

    // https://github.com/anrieff/libcpuid/blob/2e4456ae0165db3155da2e8fba92afd5c090ca1b/libcpuid/cpuid_main.c#L1112
    int32_t ext_family = __builtin_expect(family < 0xF && ffStrbufEqualS(&cpu->vendor, "AuthenticAMD"), false)
        ? family
        : family + xfamily;

    int32_t xmodel = (eax >> 16) & 0xF;
    int32_t ext_model = model + (xmodel << 4);

    const struct match_entry_t* matchtable = NULL;
    uint32_t count = 0;

    if (ffStrbufEqualS(&cpu->vendor, "GenuineIntel")) {
        matchtable = cpudb_intel;
        count = ARRAY_SIZE(cpudb_intel);
    } else if (ffStrbufEqualS(&cpu->vendor, "AuthenticAMD")) {
        matchtable = cpudb_amd;
        count = ARRAY_SIZE(cpudb_amd);
    } else if (ffStrbufEqualS(&cpu->vendor, "CentaurHauls")) {
        matchtable = cpudb_centaur;
        count = ARRAY_SIZE(cpudb_centaur);
    } else {
        return false;
    }

    const FFCPUX86MatchEntry* bestEntry = NULL;
    int32_t bestScore = -1;

    for (uint32_t i = 0; i < count; i++) {
        int score = 0;
        const FFCPUX86MatchEntry* entry = &matchtable[i];

        if (entry->family != family) {
            continue;
        }

        if (entry->model == model) {
            score += 2;
        } else if (entry->model != -1) {
            continue;
        }

        if (entry->stepping == stepping) {
            score += 2;
        } else if (entry->stepping != -1) {
            continue;
        }

        if (entry->ext_family == ext_family) {
            score += 2;
        } else if (entry->ext_family != -1) {
            continue;
        }

        if (entry->ext_model == ext_model) {
            score += 2;
        } else if (entry->ext_model != -1) {
            continue;
        }

        if (entry->brand.pattern[0] != '\0') {
            if (match_brand_pattern(entry->brand.pattern, cpu->name.chars)) {
                score += entry->brand.score;
            } else {
                continue;
            }
        }

        if (score > bestScore) {
            bestScore = score;
            bestEntry = entry;
        }
    }

    if (bestEntry && bestScore > 0) {
        cpu->codeName = bestEntry->name;
        cpu->technology = bestEntry->technology;
        return true;
    }

    return false;
}

#endif
