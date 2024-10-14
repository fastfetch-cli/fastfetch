#include "host.h"
#include "common/sysctl.h"
#include "util/smbiosHelper.h"

const char* ffDetectHost(FFHostResult* host)
{
    const char* error = NULL;
    if ((error = ffSysctlGetString(CTL_HW, HW_PRODUCT, &host->name)))
        return error;
    ffCleanUpSmbiosValue(&host->name);
    if (ffSysctlGetString(CTL_HW, HW_VENDOR, &host->vendor) == NULL)
        ffCleanUpSmbiosValue(&host->vendor);
    if (ffSysctlGetString(CTL_HW, HW_VERSION, &host->version) == NULL)
        ffCleanUpSmbiosValue(&host->version);
    if (ffSysctlGetString(CTL_HW, HW_SERIALNO, &host->serial) == NULL)
        ffCleanUpSmbiosValue(&host->serial);

    return NULL;
}
