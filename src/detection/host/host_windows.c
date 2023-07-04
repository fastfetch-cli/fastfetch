#include "host.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"

const char* ffDetectHost(FFHostResult* host)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    ffRegReadStrbuf(hKey, L"SystemProductName", &host->productName, NULL);
    ffCleanUpSmbiosValue(&host->productName);
    ffRegReadStrbuf(hKey, L"SystemFamily", &host->productFamily, NULL);
    ffCleanUpSmbiosValue(&host->productFamily);
    ffRegReadStrbuf(hKey, L"SystemVersion", &host->productVersion, NULL);
    ffCleanUpSmbiosValue(&host->productVersion);
    ffRegReadStrbuf(hKey, L"SystemSKU", &host->productSku, NULL);
    ffCleanUpSmbiosValue(&host->productSku);
    ffRegReadStrbuf(hKey, L"SystemManufacturer", &host->sysVendor, NULL);
    ffCleanUpSmbiosValue(&host->sysVendor);

    return NULL;
}
