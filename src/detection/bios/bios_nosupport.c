#include "bios.h"

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInitS(&bios->error, "Not supported on this platform");

    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);
}
