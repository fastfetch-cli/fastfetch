#include "bios.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

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

#define FF_EFIVARS_PATH_PREFIX "/sys/firmware/efi/efivars/"
#define FF_EFI_GLOBAL_GUID "8be4df61-93ca-11d2-aa0d-00e098032b8c"

const char *detectBootmgr(FFstrbuf *result)
{
    uint8_t buffer[2048];
    if (ffReadFileData(FF_EFIVARS_PATH_PREFIX "BootCurrent-" FF_EFI_GLOBAL_GUID, sizeof(buffer), buffer) != 6)
        return "Failed to read efivar: BootCurrent";

    uint16_t value = *(uint16_t *)&buffer[4];
    snprintf((char*) buffer, sizeof(buffer), FF_EFIVARS_PATH_PREFIX "Boot%04X-" FF_EFI_GLOBAL_GUID, value);

    ssize_t size = ffReadFileData((const char*) buffer, sizeof(buffer), buffer);
    if (size < 5 + (int) sizeof(FFEfiLoadOption) || size == (ssize_t) sizeof(buffer))
        return "Failed to read efivar: Boot####";

    FFEfiLoadOption *efiOption = (FFEfiLoadOption *)&buffer[4];
    uint32_t descLen = 0;
    while (efiOption->Description[descLen]) ++descLen;

    for (
        ffEfiDevicePathProtocol* filePathList = (void*) &efiOption->Description[descLen + 1];
        filePathList->Type != 0x7F; // End of Hardware Device Path
        filePathList = (void*) ((uint8_t*) filePathList + filePathList->Length))
    {
        if (filePathList->Type == 4 && filePathList->SubType == 4)
        {
            // https://uefi.org/specs/UEFI/2.10/10_Protocols_Device_Path_Protocol.html#file-path-media-device-path
            ffUcs2ToUtf8((uint16_t*) filePathList->SpecificDevicePathData, result);
            return NULL;
        }
    }

    if (!result->length) ffUcs2ToUtf8(efiOption->Description, result);

    return NULL;
}

const char *detectSecureBoot(bool* result)
{
    uint8_t buffer[5];
    if (ffReadFileData(FF_EFIVARS_PATH_PREFIX "SecureBoot-" FF_EFI_GLOBAL_GUID, sizeof(buffer), buffer) != 6)
        return "Failed to read efivar: SecureBoot";

    *result = buffer[4] == 1;
    return NULL;
}

const char *ffDetectBios(FFBiosResult *bios)
{
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_date", "/sys/class/dmi/id/bios_date", &bios->date);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_release", "/sys/class/dmi/id/bios_release", &bios->release);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_vendor", "/sys/class/dmi/id/bios_vendor", &bios->vendor);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_version", "/sys/class/dmi/id/bios_version", &bios->version);
    if (ffPathExists("/sys/firmware/efi", FF_PATHTYPE_DIRECTORY) || ffPathExists("/sys/firmware/acpi/tables/UEFI", FF_PATHTYPE_FILE))
    {
        ffStrbufSetStatic(&bios->type, "UEFI");
        detectBootmgr(&bios->bootmgr);
    }
    else
        ffStrbufSetStatic(&bios->type, "BIOS");
    detectSecureBoot(&bios->secureBoot);
    return NULL;
}
