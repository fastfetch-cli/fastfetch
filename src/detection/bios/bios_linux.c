#include "bios.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

const char *ffDetectBios(FFBiosResult *bios)
{
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_date", "/sys/class/dmi/id/bios_date", &bios->date);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_release", "/sys/class/dmi/id/bios_release", &bios->release);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_vendor", "/sys/class/dmi/id/bios_vendor", &bios->vendor);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/bios_version", "/sys/class/dmi/id/bios_version", &bios->version);
    if (ffPathExists("/sys/firmware/efi", FF_PATHTYPE_DIRECTORY) || ffPathExists("/sys/firmware/acpi/tables/UEFI", FF_PATHTYPE_FILE))
        ffStrbufSetStatic(&bios->type, "UEFI");
    else
        ffStrbufSetStatic(&bios->type, "BIOS");
    return NULL;
}
