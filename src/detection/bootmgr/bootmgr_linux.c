#include "bootmgr.h"
#include "common/io/io.h"
#include "efi.h"

#define FF_EFIVARS_PATH_PREFIX "/sys/firmware/efi/efivars/"

const char* ffDetectBootmgr(FFBootmgrResult* result)
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
            break;
        }
    }

    if (ffReadFileData(FF_EFIVARS_PATH_PREFIX "SecureBoot-" FF_EFI_GLOBAL_GUID, sizeof(buffer), buffer) == 6)
        result->secureBoot = buffer[4] == 1;

    return NULL;
}
