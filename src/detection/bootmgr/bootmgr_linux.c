#include "bootmgr.h"
#include "common/io/io.h"
#include "efi_helper.h"

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

    ffEfiFillLoadOption((FFEfiLoadOption *)&buffer[4], result);

    if (ffReadFileData(FF_EFIVARS_PATH_PREFIX "SecureBoot-" FF_EFI_GLOBAL_GUID, sizeof(buffer), buffer) == 6)
        result->secureBoot = buffer[4] == 1;

    return NULL;
}
