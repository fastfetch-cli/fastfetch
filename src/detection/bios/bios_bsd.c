#include "bios.h"

#include "common/settings.h"

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);
    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    ffSettingsGetFreeBSDKenv("smbios.bios.reldate", &bios->biosDate);
    ffSettingsGetFreeBSDKenv("smbios.bios.revision", &bios->biosRelease);
    ffSettingsGetFreeBSDKenv("smbios.bios.vendor", &bios->biosVendor);
    ffSettingsGetFreeBSDKenv("smbios.bios.version", &bios->biosVersion);
}
