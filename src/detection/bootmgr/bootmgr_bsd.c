#include "bootmgr.h"
#include "efi.h"
#include "common/io/io.h"

#include <sys/efiio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

const char* ffDetectBootmgr(FFBootmgrResult* result)
{
    FF_AUTO_CLOSE_FD int efifd = open("/dev/efi", O_RDWR);
    if (efifd < 0) return "open(/dev/efi) failed";

    uint8_t buffer[2048];
    struct efi_var_ioc ioc = {
        .vendor = { 0x8be4df61, 0x93ca, 0x11d2, 0xaa, 0x0d, { 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c } },
        .data = buffer,
    };

    ioc.datasize = sizeof(buffer);
    ioc.name = (efi_char[]){ 'B', 'o', 'o', 't', 'C', 'u', 'r', 'r', 'e', 'n', 't', '\0' };
    ioc.namesize = sizeof("BootCurrent") * 2;
    if (ioctl(efifd, EFIIOC_VAR_GET, &ioc) < 0 || ioc.datasize != 2)
        return "ioctl(EFIIOC_VAR_GET, BootCurrent) failed";

    char hex[5];
    snprintf(hex, sizeof(hex), "%04X", *(uint16_t*)buffer);
    ioc.datasize = sizeof(buffer);
    ioc.name = (efi_char[]){ 'B', 'o', 'o', 't', hex[0], hex[1], hex[2], hex[3], '\0' };
    ioc.namesize = sizeof("Boot####") * 2;
    if (ioctl(efifd, EFIIOC_VAR_GET, &ioc) < 0 || ioc.datasize == sizeof(buffer))
        return "ioctl(EFIIOC_VAR_GET, Boot####) failed";

    FFEfiLoadOption *efiOption = (FFEfiLoadOption *)buffer;
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

    ioc.name = (efi_char[]){ 'S', 'e', 'c', 'u', 'r', 'e', 'B', 'o', 'o', 't', '\0' };
    ioc.namesize = sizeof("SecureBoot") * 2;
    ioc.datasize = sizeof(buffer);
    if (ioctl(efifd, EFIIOC_VAR_GET, &ioc) == 0 && ioc.datasize == 1)
        result->secureBoot = !!buffer[0];

    return NULL;
}
