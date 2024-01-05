extern "C"
{
#include "host.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"
}
#include "util/windows/wmi.hpp"
#include "util/windows/unicode.hpp"

const char* detectWithWmi(FFHostResult* host)
{
    FFWmiQuery query(L"SELECT IdentifyingNumber, UUID FROM Win32_ComputerSystemProduct", nullptr, FFWmiNamespace::CIMV2);
    if(!query)
        return "Query WMI service failed";

    if (FFWmiRecord record = query.next())
    {
        if (auto identifyingNumber = record.get(L"IdentifyingNumber"))
            ffStrbufSetWSV(&host->serial, identifyingNumber.get<std::wstring_view>());
        if (auto uuid = record.get(L"UUID"))
            ffStrbufSetWSV(&host->uuid, uuid.get<std::wstring_view>());
        return NULL;
    }
    return "No WMI result returned";
}

const char* ffDetectHost(FFHostResult* host, FFHostOptions* options)
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

    if (options->useWmi)
        detectWithWmi(host);

    return NULL;
}
