#pragma once

#include "fastfetch.h"

// https://github.com/util-linux/util-linux/blob/master/sys-utils/lscpu-arm.c
// We use the util-linux's data but not its code. Call me if it violates util-linux's GPL license.

static const char* hwImplId2Vendor(uint32_t implId)
{
    switch (implId)
    {
    case 0x41: return "ARM";
    case 0x42: return "Broadcom";
    case 0x43: return "Cavium";
    case 0x44: return "DEC";
    case 0x46: return "FUJITSU";
    case 0x48: return "HiSilicon";
    case 0x49: return "Infineon";
    case 0x4d: return "Motorola";
    case 0x4e: return "NVIDIA";
    case 0x50: return "APM";
    case 0x51: return "Qualcomm";
    case 0x53: return "Samsung";
    case 0x56: return "Marvell";
    case 0x61: return "Apple";
    case 0x66: return "Faraday";
    case 0x69: return "Intel";
    case 0x6D: return "Microsoft";
    case 0x70: return "Phytium";
    case 0xc0: return "Ampere";
    default: return "Unknown";
    }
}

static const char* armPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x810: return "ARM810";
    case 0x920: return "ARM920";
    case 0x922: return "ARM922";
    case 0x926: return "ARM926";
    case 0x940: return "ARM940";
    case 0x946: return "ARM946";
    case 0x966: return "ARM966";
    case 0xa20: return "ARM1020";
    case 0xa22: return "ARM1022";
    case 0xa26: return "ARM1026";
    case 0xb02: return "ARM11 MPCore";
    case 0xb36: return "ARM1136";
    case 0xb56: return "ARM1156";
    case 0xb76: return "ARM1176";
    case 0xc05: return "Cortex-A5";
    case 0xc07: return "Cortex-A7";
    case 0xc08: return "Cortex-A8";
    case 0xc09: return "Cortex-A9";
    case 0xc0d: return "Cortex-A17";	/* Originally A12 */
    case 0xc0f: return "Cortex-A15";
    case 0xc0e: return "Cortex-A17";
    case 0xc14: return "Cortex-R4";
    case 0xc15: return "Cortex-R5";
    case 0xc17: return "Cortex-R7";
    case 0xc18: return "Cortex-R8";
    case 0xc20: return "Cortex-M0";
    case 0xc21: return "Cortex-M1";
    case 0xc23: return "Cortex-M3";
    case 0xc24: return "Cortex-M4";
    case 0xc27: return "Cortex-M7";
    case 0xc60: return "Cortex-M0+";
    case 0xd01: return "Cortex-A32";
    case 0xd02: return "Cortex-A34";
    case 0xd03: return "Cortex-A53";
    case 0xd04: return "Cortex-A35";
    case 0xd05: return "Cortex-A55";
    case 0xd06: return "Cortex-A65";
    case 0xd07: return "Cortex-A57";
    case 0xd08: return "Cortex-A72";
    case 0xd09: return "Cortex-A73";
    case 0xd0a: return "Cortex-A75";
    case 0xd0b: return "Cortex-A76";
    case 0xd0c: return "Neoverse-N1";
    case 0xd0d: return "Cortex-A77";
    case 0xd0e: return "Cortex-A76AE";
    case 0xd13: return "Cortex-R52";
    case 0xd15: return "Cortex-R82";
    case 0xd16: return "Cortex-R52+";
    case 0xd20: return "Cortex-M23";
    case 0xd21: return "Cortex-M33";
    case 0xd22: return "Cortex-M55";
    case 0xd23: return "Cortex-M85";
    case 0xd40: return "Neoverse-V1";
    case 0xd41: return "Cortex-A78";
    case 0xd42: return "Cortex-A78AE";
    case 0xd43: return "Cortex-A65AE";
    case 0xd44: return "Cortex-X1";
    case 0xd46: return "Cortex-A510";
    case 0xd47: return "Cortex-A710";
    case 0xd48: return "Cortex-X2";
    case 0xd49: return "Neoverse-N2";
    case 0xd4a: return "Neoverse-E1";
    case 0xd4b: return "Cortex-A78C";
    case 0xd4c: return "Cortex-X1C";
    case 0xd4d: return "Cortex-A715";
    case 0xd4e: return "Cortex-X3";
    case 0xd4f: return "Neoverse-V2";
    case 0xd80: return "Cortex-A520";
    case 0xd81: return "Cortex-A720";
    case 0xd82: return "Cortex-X4";
    case 0xd84: return "Neoverse-V3";
    case 0xd85: return "Cortex-X925";
    case 0xd87: return "Cortex-A725";
    default: return NULL;
    }
}

static const char* brcmPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x0f: return "Brahma-B15";
    case 0x100: return "Brahma-B53";
    case 0x516: return "ThunderX2";
    default: return NULL;
    }
}

static const char* decPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0xa10: return "SA110";
    case 0xa11: return "SA1100";
    default: return NULL;
    }
}

static const char* caviumPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x0a0: return "ThunderX";
    case 0x0a1: return "ThunderX-88XX";
    case 0x0a2: return "ThunderX-81XX";
    case 0x0a3: return "ThunderX-83XX";
    case 0x0af: return "ThunderX2-99xx";
    case 0x0b0: return "OcteonTX2";
    case 0x0b1: return "OcteonTX2-98XX";
    case 0x0b2: return "OcteonTX2-96XX";
    case 0x0b3: return "OcteonTX2-95XX";
    case 0x0b4: return "OcteonTX2-95XXN";
    case 0x0b5: return "OcteonTX2-95XXMM";
    case 0x0b6: return "OcteonTX2-95XXO";
    case 0x0b8: return "ThunderX3-T110";
    default: return NULL;
    }
}

static const char* apmPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x000: return "X-Gene";
    default: return NULL;
    }
}

static const char* qcomPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x001: return "Oryon";
    case 0x00f: return "Scorpion";
    case 0x02d: return "Scorpion";
    case 0x04d: return "Krait";
    case 0x06f: return "Krait";
    case 0x201: return "Kryo";
    case 0x205: return "Kryo";
    case 0x211: return "Kryo";
    case 0x800: return "Falkor-V1/Kryo";
    case 0x801: return "Kryo-V2";
    case 0x802: return "Kryo-3XX-Gold";
    case 0x803: return "Kryo-3XX-Silver";
    case 0x804: return "Kryo-4XX-Gold";
    case 0x805: return "Kryo-4XX-Silver";
    case 0xc00: return "Falkor";
    case 0xc01: return "Saphira";
    default: return NULL;
    }
}

static const char* samsungPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x001: return "Exynos-M1";
    case 0x002: return "Exynos-M3";
    case 0x003: return "Exynos-M4";
    case 0x004: return "Exynos-M5";
    default: return NULL;
    }
}

static const char* nvidiaPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x000: return "Denver";
    case 0x003: return "Denver 2";
    case 0x004: return "Carmel";
    default: return NULL;
    }
}

static const char* marvellPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x131: return "Feroceon-88FR131";
    case 0x581: return "PJ4/PJ4b";
    case 0x584: return "PJ4B-MP";
    default: return NULL;
    }
}

static const char* applePartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x000: return "Swift";
    case 0x001: return "Cyclone";
    case 0x002: return "Typhoon";
    case 0x003: return "Typhoon/Capri";
    case 0x004: return "Twister";
    case 0x005: return "Twister/Elba/Malta";
    case 0x006: return "Hurricane";
    case 0x007: return "Hurricane/Myst";
    case 0x008: return "Monsoon";
    case 0x009: return "Mistral";
    case 0x00b: return "Vortex";
    case 0x00c: return "Tempest";
    case 0x00f: return "Tempest-M9";
    case 0x010: return "Vortex/Aruba";
    case 0x011: return "Tempest/Aruba";
    case 0x012: return "Lightning";
    case 0x013: return "Thunder";
    case 0x020: return "Icestorm-A14";
    case 0x021: return "Firestorm-A14";
    case 0x022: return "Icestorm-M1";
    case 0x023: return "Firestorm-M1";
    case 0x024: return "Icestorm-M1-Pro";
    case 0x025: return "Firestorm-M1-Pro";
    case 0x026: return "Thunder-M10";
    case 0x028: return "Icestorm-M1-Max";
    case 0x029: return "Firestorm-M1-Max";
    case 0x030: return "Blizzard-A15";
    case 0x031: return "Avalanche-A15";
    case 0x032: return "Blizzard-M2";
    case 0x033: return "Avalanche-M2";
    case 0x034: return "Blizzard-M2-Pro";
    case 0x035: return "Avalanche-M2-Pro";
    case 0x036: return "Sawtooth-A16";
    case 0x037: return "Everest-A16";
    case 0x038: return "Blizzard-M2-Max";
    case 0x039: return "Avalanche-M2-Max";
    default: return NULL;
    }
}

static const char* faradayPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x526: return "FA526";
    case 0x626: return "FA626";
    default: return NULL;
    }
}

static const char* intelPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x200: return "i80200";
    case 0x210: return "PXA250A";
    case 0x212: return "PXA210A";
    case 0x242: return "i80321-400";
    case 0x243: return "i80321-600";
    case 0x290: return "PXA250B/PXA26x";
    case 0x292: return "PXA210B";
    case 0x2c2: return "i80321-400-B0";
    case 0x2c3: return "i80321-600-B0";
    case 0x2d0: return "PXA250C/PXA255/PXA26x";
    case 0x2d2: return "PXA210C";
    case 0x411: return "PXA27x";
    case 0x41c: return "IPX425-533";
    case 0x41d: return "IPX425-400";
    case 0x41f: return "IPX425-266";
    case 0x682: return "PXA32x";
    case 0x683: return "PXA930/PXA935";
    case 0x688: return "PXA30x";
    case 0x689: return "PXA31x";
    case 0xb11: return "SA1110";
    case 0xc12: return "IPX1200";
    default: return NULL;
    }
}

static const char* fujitsuPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x001: return "A64FX";
    default: return NULL;
    }
}

static const char* hisiPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0xd01: return "TaiShan-v110";	/* used in Kunpeng-920 SoC */
    case 0xd02: return "TaiShan-v120";	/* used in Kirin 990A and 9000S SoCs */
    case 0xd40: return "Cortex-A76";	/* HiSilicon uses this ID though advertises A76 */
    case 0xd41: return "Cortex-A77";	/* HiSilicon uses this ID though advertises A77 */
    default: return NULL;
    }
}

static const char* amperePartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0xac3: return "Ampere-1";
    case 0xac4: return "Ampere-1a";
    default: return NULL;
    }
}

static const char* ftPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0x303: return "FTC310";
    case 0x660: return "FTC660";
    case 0x661: return "FTC661";
    case 0x662: return "FTC662";
    case 0x663: return "FTC663";
    case 0x664: return "FTC664";
    case 0x862: return "FTC862";
    default: return NULL;
    }
}

static const char* msPartId2name(uint32_t partId)
{
    switch (partId)
    {
    case 0xd49: return "Azure-Cobalt-100";
    default: return NULL;
    }
}
