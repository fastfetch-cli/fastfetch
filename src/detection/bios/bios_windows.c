#include "bios.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"

const char* ffDetectBios(FFBiosResult* bios)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    if(!ffRegReadStrbuf(hKey, L"BIOSVersion", &bios->version, NULL))
        return "\"HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS\\BIOSVersion\" doesn't exist";

    ffCleanUpSmbiosValue(&bios->version);
    ffRegReadStrbuf(hKey, L"BIOSVendor", &bios->vendor, NULL);
    ffCleanUpSmbiosValue(&bios->vendor);
    ffRegReadStrbuf(hKey, L"BIOSReleaseDate", &bios->date, NULL);
    ffCleanUpSmbiosValue(&bios->date);

    uint32_t major, minor;
    if(
        ffRegReadUint(hKey, L"BiosMajorRelease", &major, NULL) &&
        ffRegReadUint(hKey, L"BiosMinorRelease", &minor, NULL)
    )
        ffStrbufAppendF(&bios->release, "%u.%u", (unsigned)major, (unsigned)minor);

    return NULL;
}
