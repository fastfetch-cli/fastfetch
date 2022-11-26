#include "host.h"
#include "util/windows/registry.h"

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

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, &host->error))
        return;

    ffRegReadStrbuf(hKey, L"SystemProductName", &host->productName, NULL);
    ffRegReadStrbuf(hKey, L"SystemFamily", &host->productFamily, NULL);
    ffRegReadStrbuf(hKey, L"SystemVersion", &host->productVersion, NULL);
    ffRegReadStrbuf(hKey, L"SystemSKU", &host->productSku, NULL);
    ffRegReadStrbuf(hKey, L"SystemManufacturer", &host->sysVendor, NULL);
}
