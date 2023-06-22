#include "chassis.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectChassis(FFChassisResult* result)
{
    // Unlike other platforms, `smbios.chassis.type` return display string directly on my machine
    ffSettingsGetFreeBSDKenv("smbios.chassis.type", &result->chassisType);
    ffCleanUpSmbiosValue(&result->chassisType);
    ffSettingsGetFreeBSDKenv("smbios.chassis.maker", &result->chassisVendor);
    ffCleanUpSmbiosValue(&result->chassisVendor);
    ffSettingsGetFreeBSDKenv("smbios.chassis.version", &result->chassisVersion);
    ffCleanUpSmbiosValue(&result->chassisVersion);
    return NULL;
}
