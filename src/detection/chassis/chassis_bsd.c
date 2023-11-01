#include "chassis.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectChassis(FFChassisResult* result, FF_MAYBE_UNUSED FFChassisOptions* options)
{
    // Unlike other platforms, `smbios.chassis.type` return display string directly on my machine
    ffSettingsGetFreeBSDKenv("smbios.chassis.type", &result->type);
    ffCleanUpSmbiosValue(&result->type);
    ffSettingsGetFreeBSDKenv("smbios.chassis.maker", &result->vendor);
    ffCleanUpSmbiosValue(&result->vendor);
    ffSettingsGetFreeBSDKenv("smbios.chassis.version", &result->version);
    ffCleanUpSmbiosValue(&result->version);
    return NULL;
}
