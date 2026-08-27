#include "host.h"
#include "common/sysctl.h"
#include "common/smbios.h"

const char* ffDetectHost(FFHostResult* host) {
    const char* error = nullptr;
    if ((error = ffSysctlGetString(CTL_HW, HW_PRODUCT, &host->name))) {
        return error;
    }
    ffCleanUpSmbiosValue(&host->name);
    if (ffSysctlGetString(CTL_HW, HW_VENDOR, &host->vendor) == nullptr) {
        ffCleanUpSmbiosValue(&host->vendor);
    }
    if (ffSysctlGetString(CTL_HW, HW_VERSION, &host->version) == nullptr) {
        ffCleanUpSmbiosValue(&host->version);
    }
    if (ffSysctlGetString(CTL_HW, HW_SERIALNO, &host->serial) == nullptr) {
        ffCleanUpSmbiosValue(&host->serial);
    }

    return nullptr;
}
