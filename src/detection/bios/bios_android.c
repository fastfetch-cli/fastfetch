#include "bios.h"
#include "common/settings.h"

const char* ffDetectBios(FFBiosResult* bios)
{
    if (!ffSettingsGetAndroidProperty("ro.bootloader", &bios->version))
        ffSettingsGetAndroidProperty("ro.boot.bootloader", &bios->version);

    if (ffStrbufIgnCaseEqualS(&bios->version, "unknown"))
        ffStrbufClear(&bios->version);

    return NULL;
}
