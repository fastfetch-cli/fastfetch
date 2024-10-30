#include "bios.h"
#include "common/sysctl.h"
#include "util/smbiosHelper.h"

const char* ffDetectBios(FFBiosResult* bios)
{
    if (ffSysctlGetString("machdep.dmi.bios-date", &bios->date) == NULL)
        ffCleanUpSmbiosValue(&bios->date);
    if (ffSysctlGetString("machdep.dmi.bios-version", &bios->version) == NULL)
        ffCleanUpSmbiosValue(&bios->version);
    if (ffSysctlGetString("machdep.dmi.bios-vendor", &bios->vendor) == NULL)
        ffCleanUpSmbiosValue(&bios->vendor);
    ffSysctlGetString("machdep.bootmethod", &bios->type);
    return NULL;
}
