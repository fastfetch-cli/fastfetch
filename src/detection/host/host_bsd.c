#include "host.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectHost(FFHostResult* host)
{
    ffSettingsGetFreeBSDKenv("smbios.system.product", &host->productName);
    ffCleanUpSmbiosValue(&host->productName);
    ffSettingsGetFreeBSDKenv("smbios.system.family", &host->productFamily);
    ffCleanUpSmbiosValue(&host->productFamily);
    ffSettingsGetFreeBSDKenv("smbios.system.version", &host->productVersion);
    ffCleanUpSmbiosValue(&host->productVersion);
    ffSettingsGetFreeBSDKenv("smbios.system.sku", &host->productSku);
    ffCleanUpSmbiosValue(&host->productSku);
    ffSettingsGetFreeBSDKenv("smbios.system.maker", &host->sysVendor);
    ffCleanUpSmbiosValue(&host->sysVendor);

    return NULL;
}
