#include "bootmgr.h"
#include "efi_helper.h"
#include "common/io/io.h"

#ifdef __OpenBSD__
    #include <dev/efi/efiio.h>
#else
    #include <sys/efiio.h>
#endif
#include <sys/ioctl.h>
#include <fcntl.h>

#ifdef __NetBSD__
    typedef uint16_t efi_char;
#endif

#ifndef EFI_GLOBAL_VARIABLE
    #define EFI_GLOBAL_VARIABLE { 0x8be4df61, 0x93ca, 0x11d2, 0xaa, 0x0d, { 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c } }
#endif

const char* ffDetectBootmgr(FFBootmgrResult* result)
{
    FF_AUTO_CLOSE_FD int efifd = open("/dev/efi", O_RDWR | O_CLOEXEC);
    if (efifd < 0) return "open(/dev/efi) failed";

    uint8_t buffer[2048];
    struct efi_var_ioc ioc = {
        .vendor = EFI_GLOBAL_VARIABLE,
        .data = buffer,
    };

    ioc.datasize = sizeof(buffer);
    ioc.name = (efi_char[]){ 'B', 'o', 'o', 't', 'C', 'u', 'r', 'r', 'e', 'n', 't', '\0' };
    ioc.namesize = sizeof("BootCurrent") * 2;
    if (ioctl(efifd, EFIIOC_VAR_GET, &ioc) < 0 || ioc.datasize != 2)
        return "ioctl(EFIIOC_VAR_GET, BootCurrent) failed";

    result->order = *(uint16_t*)buffer;

    unsigned char hex[5];
    snprintf((char*) hex, sizeof(hex), "%04X", result->order);
    ioc.datasize = sizeof(buffer);
    ioc.name = (efi_char[]){ 'B', 'o', 'o', 't', hex[0], hex[1], hex[2], hex[3], '\0' };
    ioc.namesize = sizeof("Boot####") * 2;
    if (ioctl(efifd, EFIIOC_VAR_GET, &ioc) < 0 || ioc.datasize == sizeof(buffer))
        return "ioctl(EFIIOC_VAR_GET, Boot####) failed";

    ffEfiFillLoadOption((FFEfiLoadOption *)buffer, result);

    ioc.name = (efi_char[]){ 'S', 'e', 'c', 'u', 'r', 'e', 'B', 'o', 'o', 't', '\0' };
    ioc.namesize = sizeof("SecureBoot") * 2;
    ioc.datasize = sizeof(buffer);
    if (ioctl(efifd, EFIIOC_VAR_GET, &ioc) == 0 && ioc.datasize == 1)
        result->secureBoot = !!buffer[0];

    return NULL;
}
