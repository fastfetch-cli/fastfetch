#include "chassis.h"
#include "common/sysctl.h"
#include "util/smbiosHelper.h"

const char* ffDetectChassis(FFChassisResult* chassis)
{
    if (ffSysctlGetString("machdep.dmi.chassis-type", &chassis->type) == NULL)
        ffCleanUpSmbiosValue(&chassis->type);
    if (ffSysctlGetString("machdep.dmi.chassis-version", &chassis->version) == NULL)
        ffCleanUpSmbiosValue(&chassis->version);
    if (ffSysctlGetString("machdep.dmi.chassis-vendor", &chassis->vendor) == NULL)
        ffCleanUpSmbiosValue(&chassis->vendor);

    return NULL;
}
