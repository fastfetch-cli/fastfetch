#include "chassis.h"
#include "common/settings.h"

const char* ffDetectChassis(FFChassisResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.chassis.type", &result->chassisType);
    ffSettingsGetFreeBSDKenv("smbios.chassis.maker", &result->chassisVendor);
    ffSettingsGetFreeBSDKenv("smbios.chassis.version", &result->chassisVersion);
    return NULL;
}
