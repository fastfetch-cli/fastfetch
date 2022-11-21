#include "bios.h"
#include "util/windows/register.h"

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);

    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, &bios->error))
        return;

    if(!ffRegReadStrbuf(hKey, "BIOSVersion", &bios->biosRelease, &bios->error))
        return;
    ffRegReadStrbuf(hKey, "BIOSVendor", &bios->biosVendor, NULL);
    ffRegReadStrbuf(hKey, "BIOSReleaseDate", &bios->biosDate, NULL);

    uint32_t major, minor;
    if(
        ffRegReadUint(hKey, "BiosMajorRelease", &major, NULL) &&
        ffRegReadUint(hKey, "BiosMinorRelease", &minor, NULL)
    )
        ffStrbufAppendF(&bios->biosVersion, "%u.%u", (unsigned)major, (unsigned)minor);
}
