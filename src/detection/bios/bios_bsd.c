#include "bios.h"

#include <kenv.h>

static void kenvLookup(const char* name, FFstrbuf* result)
{
    ffStrbufEnsureFree(result, 255);
    int len = kenv(KENV_GET, name, result->chars, 256);
    if(len > 0)
        result->length = (uint32_t) len;
}

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);
    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    //https://wiki.ghostbsd.org/index.php/Kenv
    kenvLookup("smbios.bios.reldate", &bios->biosDate);
    kenvLookup("smbios.system.product", &bios->biosRelease);
    kenvLookup("smbios.bios.vendor", &bios->biosVendor);
    kenvLookup("smbios.bios.version", &bios->biosVersion);
}
