#include "host.h"
#include "common/sysctl.h"
#include "common/smbios.h"

const char* ffDetectHost(FFHostResult* host) {
    const char* error = nullptr;
    if ((error = ffSysctlGetString("machdep.dmi.system-product", &host->name))) {
        return error;
    }
    ffCleanUpSmbiosValue(&host->name);
    if (ffSysctlGetString("machdep.dmi.system-vendor", &host->vendor) == nullptr) {
        ffCleanUpSmbiosValue(&host->vendor);
    }
    if (ffSysctlGetString("machdep.dmi.system-version", &host->version) == nullptr) {
        ffCleanUpSmbiosValue(&host->version);
    }
    if (ffSysctlGetString("machdep.dmi.system-serial", &host->serial) == nullptr) {
        ffCleanUpSmbiosValue(&host->serial);
    }
    if (ffSysctlGetString("machdep.dmi.system-uuid", &host->uuid) == nullptr) {
        ffCleanUpSmbiosValue(&host->uuid);
    }

    return nullptr;
}
