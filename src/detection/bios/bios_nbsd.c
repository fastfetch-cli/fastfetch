#include "bios.h"
#include "common/sysctl.h"
#include "common/smbios.h"
#include "common/io.h"

const char* ffDetectBios(FFBiosResult* bios) {
    if (ffSysctlGetString("machdep.dmi.bios-date", &bios->date) == NULL) {
        ffCleanUpSmbiosValue(&bios->date);
    }
    if (ffSysctlGetString("machdep.dmi.bios-version", &bios->version) == NULL) {
        ffCleanUpSmbiosValue(&bios->version);
    }
    if (ffSysctlGetString("machdep.dmi.bios-vendor", &bios->vendor) == NULL) {
        ffCleanUpSmbiosValue(&bios->vendor);
    }
    if (ffSysctlGetString("machdep.bootmethod", &bios->type) != NULL) {
        ffStrbufSetStatic(&bios->type, ffPathExists("/dev/efi", FF_PATHTYPE_FILE) ? "UEFI" : "BIOS");
    }
    return NULL;
}
