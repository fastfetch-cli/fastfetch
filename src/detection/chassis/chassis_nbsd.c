#include "chassis.h"
#include "common/sysctl.h"
#include "common/smbios.h"

const char* ffDetectChassis(FFChassisResult* chassis) {
    if (ffSysctlGetString("machdep.dmi.chassis-type", &chassis->type) == nullptr) {
        ffCleanUpSmbiosValue(&chassis->type);
    }
    if (ffSysctlGetString("machdep.dmi.chassis-version", &chassis->version) == nullptr) {
        ffCleanUpSmbiosValue(&chassis->version);
    }
    if (ffSysctlGetString("machdep.dmi.chassis-vendor", &chassis->vendor) == nullptr) {
        ffCleanUpSmbiosValue(&chassis->vendor);
    }
    if (ffSysctlGetString("machdep.dmi.chassis-serial", &chassis->serial) == nullptr) {
        ffCleanUpSmbiosValue(&chassis->serial);
    }

    return nullptr;
}
