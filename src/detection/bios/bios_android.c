#include "bios.h"
#include "util/settings.h"

const char* ffDetectBios(FFBiosResult* bios)
{
    if (!ffSettingsGetAndroidProperty("ro.bootloader", &bios->version))
        ffSettingsGetAndroidProperty("ro.boot.bootloader", &bios->version);

    if (ffStrbufIgnCaseEqualS(&bios->version, "unknown"))
        ffStrbufClear(&bios->version);

    ffStrbufSetStatic(&bios->type, "Bootloader");

    return NULL;
}
