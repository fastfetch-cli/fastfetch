#include "fastfetch.h"

// https://uefi.org/specs/UEFI/2.10/10_Protocols_Device_Path_Protocol.html#generic-device-path-structures
typedef struct ffEfiDevicePathProtocol
{
    uint8_t Type;
    uint8_t SubType;
    uint16_t Length;
    uint8_t SpecificDevicePathData[];
} ffEfiDevicePathProtocol;

// https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html#load-options
typedef struct FFEfiLoadOption
{
    uint32_t Attributes;
    uint16_t FilePathListLength;
    uint16_t Description[];
    // ffEfiDevicePathProtocol FilePathList[];
    // uint8_t OptionalData[];
} FFEfiLoadOption;

uint8_t ffEvBits(uint16_t val, uint8_t mask, uint8_t shift)
{
    return (uint8_t) ((val & (mask << shift)) >> shift);
}

static void ffUcs2ToUtf8(const uint16_t *const chars, FFstrbuf* result)
{
    for (uint32_t i = 0; chars[i]; i++)
    {
        if (chars[i] <= 0x007f)
            ffStrbufAppendC(result, (char) chars[i]);
        else if (chars[i] > 0x007f && chars[i] <= 0x07ff)
        {
            ffStrbufAppendC(result, (char) (0xc0 | ffEvBits(chars[i], 0x1f, 6)));
            ffStrbufAppendC(result, (char) (0x80 | ffEvBits(chars[i], 0x3f, 0)));
        }
        else
        {
            ffStrbufAppendC(result, (char) (0xe0 | ffEvBits(chars[i], 0xf, 12)));
            ffStrbufAppendC(result, (char) (0x80 | ffEvBits(chars[i], 0x3f, 6)));
            ffStrbufAppendC(result, (char) (0x80 | ffEvBits(chars[i], 0x3f, 0)));
        }
    }
}

#define FF_EFI_GLOBAL_GUID "8be4df61-93ca-11d2-aa0d-00e098032b8c"
