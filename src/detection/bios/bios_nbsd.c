#include "bios.h"
#include "common/sysctl.h"
#include "common/smbios.h"
#include "common/io.h"

const char* ffDetectBios(FFBiosResult* bios) {
    if (ffSysctlGetString("machdep.dmi.bios-date", &bios->date) == nullptr) {
        ffCleanUpSmbiosValue(&bios->date);
    }
    if (ffSysctlGetString("machdep.dmi.bios-version", &bios->version) == nullptr) {
        ffCleanUpSmbiosValue(&bios->version);
    }
    if (ffSysctlGetString("machdep.dmi.bios-vendor", &bios->vendor) == nullptr) {
        ffCleanUpSmbiosValue(&bios->vendor);
    }
    if (ffSysctlGetString("machdep.bootmethod", &bios->type) != nullptr) {
        ffStrbufSetStatic(&bios->type, ffPathExists("/dev/efi", FF_PATHTYPE_FILE) ? "UEFI" : "BIOS");
    }
    return nullptr;
}
