#include "efi_helper.h"

static inline uint8_t evBits(uint16_t val, uint8_t mask, uint8_t shift)
{
    return (uint8_t) ((val & (mask << shift)) >> shift);
}

static void ffEfiUcs2ToUtf8(const uint16_t *const chars, FFstrbuf* result)
{
    for (uint32_t i = 0; chars[i]; i++)
    {
        if (chars[i] <= 0x007f)
            ffStrbufAppendC(result, (char) chars[i]);
        else if (chars[i] > 0x007f && chars[i] <= 0x07ff)
        {
            ffStrbufAppendC(result, (char) (0xc0 | evBits(chars[i], 0x1f, 6)));
            ffStrbufAppendC(result, (char) (0x80 | evBits(chars[i], 0x3f, 0)));
        }
        else
        {
            ffStrbufAppendC(result, (char) (0xe0 | evBits(chars[i], 0xf, 12)));
            ffStrbufAppendC(result, (char) (0x80 | evBits(chars[i], 0x3f, 6)));
            ffStrbufAppendC(result, (char) (0x80 | evBits(chars[i], 0x3f, 0)));
        }
    }
}

bool ffEfiFillLoadOption(const FFEfiLoadOption* efiOption, FFBootmgrResult* result)
{
    uint32_t descLen = 0;
    while (efiOption->Description[descLen]) ++descLen;

    if (descLen)
        ffEfiUcs2ToUtf8(efiOption->Description, &result->name);

    for (
        ffEfiDevicePathProtocol* filePathList = (void*) &efiOption->Description[descLen + 1];
        filePathList->Type != 0x7F; // End of Hardware Device Path
        filePathList = (void*) ((uint8_t*) filePathList + filePathList->Length))
    {
        if (filePathList->Type == 4 && filePathList->SubType == 4)
        {
            // https://uefi.org/specs/UEFI/2.10/10_Protocols_Device_Path_Protocol.html#file-path-media-device-path
            ffEfiUcs2ToUtf8((uint16_t*) filePathList->SpecificDevicePathData, &result->firmware);
            return true;
        }
    }

    return false;
}
