#include "host.h"
#include "util/windows/registry.h"

const char* ffDetectHost(FFHostResult* host)
{
    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);

    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    ffRegReadStrbuf(hKey, L"SystemProductName", &host->productName, NULL);
    ffRegReadStrbuf(hKey, L"SystemFamily", &host->productFamily, NULL);
    ffRegReadStrbuf(hKey, L"SystemVersion", &host->productVersion, NULL);
    ffRegReadStrbuf(hKey, L"SystemSKU", &host->productSku, NULL);
    ffRegReadStrbuf(hKey, L"SystemManufacturer", &host->sysVendor, NULL);

    return NULL;
}
