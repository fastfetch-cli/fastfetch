#include "host.h"
#include "common/settings.h"

const char* ffDetectHost(FFHostResult* host)
{
    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);
    ffSettingsGetFreeBSDKenv("smbios.system.product", &host->productName);
    ffSettingsGetFreeBSDKenv("smbios.system.family", &host->productFamily);
    ffSettingsGetFreeBSDKenv("smbios.system.version", &host->productVersion);
    ffSettingsGetFreeBSDKenv("smbios.system.sku", &host->productSku);
    ffSettingsGetFreeBSDKenv("smbios.system.maker", &host->sysVendor);

    return NULL;
}
