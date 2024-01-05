#include "host.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"

const char* ffDetectHost(FFHostResult* host)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    ffRegReadStrbuf(hKey, L"SystemProductName", &host->name, NULL);
    ffCleanUpSmbiosValue(&host->name);
    ffRegReadStrbuf(hKey, L"SystemFamily", &host->family, NULL);
    ffCleanUpSmbiosValue(&host->family);
    ffRegReadStrbuf(hKey, L"SystemVersion", &host->version, NULL);
    ffCleanUpSmbiosValue(&host->version);
    ffRegReadStrbuf(hKey, L"SystemSKU", &host->sku, NULL);
    ffCleanUpSmbiosValue(&host->sku);
    ffRegReadStrbuf(hKey, L"SystemManufacturer", &host->vendor, NULL);
    ffCleanUpSmbiosValue(&host->vendor);

    return NULL;
}
