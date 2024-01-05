#include "host.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectHost(FFHostResult* host)
{
    ffSettingsGetFreeBSDKenv("smbios.system.product", &host->name);
    ffCleanUpSmbiosValue(&host->name);
    ffSettingsGetFreeBSDKenv("smbios.system.family", &host->family);
    ffCleanUpSmbiosValue(&host->family);
    ffSettingsGetFreeBSDKenv("smbios.system.version", &host->version);
    ffCleanUpSmbiosValue(&host->version);
    ffSettingsGetFreeBSDKenv("smbios.system.sku", &host->sku);
    ffCleanUpSmbiosValue(&host->sku);
    ffSettingsGetFreeBSDKenv("smbios.system.serial", &host->serial);
    ffCleanUpSmbiosValue(&host->serial);
    ffSettingsGetFreeBSDKenv("smbios.system.uuid", &host->uuid);
    ffCleanUpSmbiosValue(&host->uuid);
    ffSettingsGetFreeBSDKenv("smbios.system.maker", &host->vendor);
    ffCleanUpSmbiosValue(&host->vendor);

    return NULL;
}
