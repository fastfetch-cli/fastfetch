#include "bluetoothradio.h"

// https://github.com/ziglang/zig/blob/a84951465b409495095a9598db0cae745f34fa7b/lib/libc/include/any-windows-any/bthdef.h#L187-L236

#define BTH_MFG_ERICSSON 0
#define BTH_MFG_NOKIA 1
#define BTH_MFG_INTEL 2
#define BTH_MFG_IBM 3
#define BTH_MFG_TOSHIBA 4
#define BTH_MFG_3COM 5
#define BTH_MFG_MICROSOFT 6
#define BTH_MFG_LUCENT 7
#define BTH_MFG_MOTOROLA 8
#define BTH_MFG_INFINEON 9
#define BTH_MFG_CSR 10
#define BTH_MFG_SILICONWAVE 11
#define BTH_MFG_DIGIANSWER 12
#define BTH_MFG_TI 13
#define BTH_MFG_PARTHUS 14
#define BTH_MFG_BROADCOM 15
#define BTH_MFG_MITEL 16
#define BTH_MFG_WIDCOMM 17
#define BTH_MFG_ZEEVO 18
#define BTH_MFG_ATMEL 19
#define BTH_MFG_MITSIBUSHI 20
#define BTH_MFG_RTX_TELECOM 21
#define BTH_MFG_KC_TECHNOLOGY 22
#define BTH_MFG_NEWLOGIC 23
#define BTH_MFG_TRANSILICA 24
#define BTH_MFG_ROHDE_SCHWARZ 25
#define BTH_MFG_TTPCOM 26
#define BTH_MFG_SIGNIA 27
#define BTH_MFG_CONEXANT 28
#define BTH_MFG_QUALCOMM 29
#define BTH_MFG_INVENTEL 30
#define BTH_MFG_AVM_BERLIN 31
#define BTH_MFG_BANDSPEED 32
#define BTH_MFG_MANSELLA 33
#define BTH_MFG_NEC 34
#define BTH_MFG_WAVEPLUS_TECHNOLOGY_CO 35
#define BTH_MFG_ALCATEL 36
#define BTH_MFG_PHILIPS_SEMICONDUCTOR 37
#define BTH_MFG_C_TECHNOLOGIES 38
#define BTH_MFG_OPEN_INTERFACE 39
#define BTH_MFG_RF_MICRO_DEVICES 40
#define BTH_MFG_HITACHI 41
#define BTH_MFG_SYMBOL_TECHNOLOGIES 42
#define BTH_MFG_TENOVIS 43
#define BTH_MFG_MACRONIX_INTERNATIONAL 44
#define BTH_MFG_MARVELL 72
#define BTH_MFG_APPLE 76
#define BTH_MFG_NORDIC_SEMICONDUCTORS_ASA 89
#define BTH_MFG_ARUBA_NETWORKS 283
#define BTH_MFG_INTERNAL_USE 65535

const char* ffBluetoothRadioGetVendor(uint32_t manufacturerId)
{
    switch (manufacturerId)
    {
        case BTH_MFG_ERICSSON: return "Ericsson";
        case BTH_MFG_NOKIA: return "Nokia";
        case BTH_MFG_INTEL: return "Intel";
        case BTH_MFG_IBM: return "IBM";
        case BTH_MFG_TOSHIBA: return "Toshiba";
        case BTH_MFG_3COM: return "3Com";
        case BTH_MFG_MICROSOFT: return "Microsoft";
        case BTH_MFG_LUCENT: return "Lucent";
        case BTH_MFG_MOTOROLA: return "Motorola";
        case BTH_MFG_INFINEON: return "Infineon";
        case BTH_MFG_CSR: return "CSR";
        case BTH_MFG_SILICONWAVE: return "Silicon-Wave";
        case BTH_MFG_DIGIANSWER: return "Digi-Answer";
        case BTH_MFG_TI: return "Ti";
        case BTH_MFG_PARTHUS: return "Parthus";
        case BTH_MFG_BROADCOM: return "Broadcom";
        case BTH_MFG_MITEL: return "Mitel";
        case BTH_MFG_WIDCOMM: return "Widcomm";
        case BTH_MFG_ZEEVO: return "Zeevo";
        case BTH_MFG_ATMEL: return "Atmel";
        case BTH_MFG_MITSIBUSHI: return "Mitsubishi";
        case BTH_MFG_RTX_TELECOM: return "RTX Telecom";
        case BTH_MFG_KC_TECHNOLOGY: return "KC Technology";
        case BTH_MFG_NEWLOGIC: return "Newlogic";
        case BTH_MFG_TRANSILICA: return "Transilica";
        case BTH_MFG_ROHDE_SCHWARZ: return "Rohde-Schwarz";
        case BTH_MFG_TTPCOM: return "TTPCom";
        case BTH_MFG_SIGNIA: return "Signia";
        case BTH_MFG_CONEXANT: return "Conexant";
        case BTH_MFG_QUALCOMM: return "Qualcomm";
        case BTH_MFG_INVENTEL: return "Inventel";
        case BTH_MFG_AVM_BERLIN: return "AVM Berlin";
        case BTH_MFG_BANDSPEED: return "Bandspeed";
        case BTH_MFG_MANSELLA: return "Mansella";
        case BTH_MFG_NEC: return "NEC";
        case BTH_MFG_WAVEPLUS_TECHNOLOGY_CO: return "Waveplus";
        case BTH_MFG_ALCATEL: return "Alcatel";
        case BTH_MFG_PHILIPS_SEMICONDUCTOR: return "Philips Semiconductors";
        case BTH_MFG_C_TECHNOLOGIES: return "C Technologies";
        case BTH_MFG_OPEN_INTERFACE: return "Open Interface";
        case BTH_MFG_RF_MICRO_DEVICES: return "RF Micro Devices";
        case BTH_MFG_HITACHI: return "Hitachi";
        case BTH_MFG_SYMBOL_TECHNOLOGIES: return "Symbol Technologies";
        case BTH_MFG_TENOVIS: return "Tenovis";
        case BTH_MFG_MACRONIX_INTERNATIONAL: return "Macronix International";
        case BTH_MFG_MARVELL: return "Marvell";
        case BTH_MFG_APPLE: return "Apple";
        case BTH_MFG_NORDIC_SEMICONDUCTORS_ASA: return "Nordic Semiconductor ASA";
        case BTH_MFG_ARUBA_NETWORKS: return "Aruba Networks";
        case BTH_MFG_INTERNAL_USE: return "Internal Use";
        default: return "Unknown";
    }
}
