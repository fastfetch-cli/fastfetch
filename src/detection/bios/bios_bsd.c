#include "bios.h"

#include "common/settings.h"

const char* ffDetectBios(FFBiosResult* bios)
{
    ffSettingsGetFreeBSDKenv("smbios.bios.reldate", &bios->biosDate);
    ffSettingsGetFreeBSDKenv("smbios.bios.revision", &bios->biosRelease);
    ffSettingsGetFreeBSDKenv("smbios.bios.vendor", &bios->biosVendor);
    ffSettingsGetFreeBSDKenv("smbios.bios.version", &bios->biosVersion);
    return NULL;
}
