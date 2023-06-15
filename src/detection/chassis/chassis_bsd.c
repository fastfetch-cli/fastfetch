#include "chassis.h"
#include "common/settings.h"

void ffDetectChassis(FFChassisResult* result)
{
    ffStrbufInit(&result->error);
    ffStrbufInit(&result->chassisType);
    ffStrbufInit(&result->chassisVendor);
    ffStrbufInit(&result->chassisVersion);
    ffSettingsGetFreeBSDKenv("smbios.chassis.type", &result->chassisType);
    ffSettingsGetFreeBSDKenv("smbios.chassis.maker", &result->chassisVendor);
    ffSettingsGetFreeBSDKenv("smbios.chassis.version", &result->chassisVersion);
}
