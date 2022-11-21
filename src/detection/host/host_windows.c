#include "host.h"
#include "util/windows/register.h"

void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);
    ffStrbufInit(&host->chassisType);
    ffStrbufInit(&host->chassisVendor);
    ffStrbufInit(&host->chassisVersion);

    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, &host->error))
        return;

    ffRegReadStrbuf(hKey, "SystemProductName", &host->productName, NULL);
    ffRegReadStrbuf(hKey, "SystemFamily", &host->productFamily, NULL);
    ffRegReadStrbuf(hKey, "SystemVersion", &host->productVersion, NULL);
    ffRegReadStrbuf(hKey, "SystemSKU", &host->productSku, NULL);
    ffRegReadStrbuf(hKey, "SystemManufacturer", &host->sysVendor, NULL);
}
