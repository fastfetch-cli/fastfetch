#include "bios.h"
#include "util/windows/registry.h"

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);

    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, &bios->error))
        return;

    if(!ffRegReadStrbuf(hKey, L"BIOSVersion", &bios->biosRelease, &bios->error))
        return;
    ffRegReadStrbuf(hKey, L"BIOSVendor", &bios->biosVendor, NULL);
    ffRegReadStrbuf(hKey, L"BIOSReleaseDate", &bios->biosDate, NULL);

    uint32_t major, minor;
    if(
        ffRegReadUint(hKey, L"BiosMajorRelease", &major, NULL) &&
        ffRegReadUint(hKey, L"BiosMinorRelease", &minor, NULL)
    )
        ffStrbufAppendF(&bios->biosVersion, "%u.%u", (unsigned)major, (unsigned)minor);
}
