#include "bios.h"

#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectBios(FFBiosResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.bios.reldate", &result->biosDate);
    ffCleanUpSmbiosValue(&result->biosDate);
    ffSettingsGetFreeBSDKenv("smbios.bios.revision", &result->biosRelease);
    ffCleanUpSmbiosValue(&result->biosRelease);
    ffSettingsGetFreeBSDKenv("smbios.bios.vendor", &result->biosVendor);
    ffCleanUpSmbiosValue(&result->biosVendor);
    ffSettingsGetFreeBSDKenv("smbios.bios.version", &result->biosVersion);
    ffCleanUpSmbiosValue(&result->biosVersion);
    return NULL;
}
