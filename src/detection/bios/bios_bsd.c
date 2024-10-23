#include "bios.h"

#include "common/settings.h"
#include "common/sysctl.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

const char* ffDetectBios(FFBiosResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.bios.reldate", &result->date);
    ffCleanUpSmbiosValue(&result->date);
    ffSettingsGetFreeBSDKenv("smbios.bios.revision", &result->release);
    ffCleanUpSmbiosValue(&result->release);
    ffSettingsGetFreeBSDKenv("smbios.bios.vendor", &result->vendor);
    ffCleanUpSmbiosValue(&result->vendor);
    ffSettingsGetFreeBSDKenv("smbios.bios.version", &result->version);
    ffCleanUpSmbiosValue(&result->version);
    ffSysctlGetString("machdep.bootmethod", &result->type);

    if (result->type.length == 0)
    {
        if (ffSettingsGetFreeBSDKenv("loader.efi", &result->type))
            ffStrbufSetStatic(&result->type, ffStrbufEqualS(&result->type, "1") ? "UEFI" : "BIOS");
        else
        {
            ffStrbufSetStatic(&result->type,
                ffPathExists("/dev/efi" /*efidev*/, FF_PATHTYPE_FILE) ||
                ffPathExists("/boot/efi/efi/" /*efi partition*/, FF_PATHTYPE_DIRECTORY)
                    ? "UEFI" : "BIOS");
        }
    }
    return NULL;
}
