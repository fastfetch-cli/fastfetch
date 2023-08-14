#include "bios.h"

#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectBios(FFBiosResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.bios.reldate", &result->date);
    ffCleanUpSmbiosValue(&result->date);
    ffSettingsGetFreeBSDKenv("smbios.bios.revision", &result->release);
    ffCleanUpSmbiosValue(&result->release);
    ffSettingsGetFreeBSDKenv("smbios.bios.vendor", &result->vendor);
    ffCleanUpSmbiosValue(&result->vendor);
    ffSettingsGetFreeBSDKenv("smbios.bios.version", &result->version);
    ffCleanUpSmbiosValue(&result->version);
    return NULL;
}
