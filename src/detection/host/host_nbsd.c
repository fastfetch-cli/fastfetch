#include "host.h"
#include "common/sysctl.h"
#include "util/smbiosHelper.h"

const char* ffDetectHost(FFHostResult* host)
{
    const char* error = NULL;
    if ((error = ffSysctlGetString("machdep.dmi.system-product", &host->name)))
        return error;
    ffCleanUpSmbiosValue(&host->name);
    if (ffSysctlGetString("machdep.dmi.system-vendor", &host->vendor) == NULL)
        ffCleanUpSmbiosValue(&host->vendor);
    if (ffSysctlGetString("machdep.dmi.system-version", &host->version) == NULL)
        ffCleanUpSmbiosValue(&host->version);
    if (ffSysctlGetString("machdep.dmi.system-serial", &host->serial) == NULL)
        ffCleanUpSmbiosValue(&host->serial);
    if (ffSysctlGetString("machdep.dmi.system-uuid", &host->uuid) == NULL)
        ffCleanUpSmbiosValue(&host->uuid);

    return NULL;
}
