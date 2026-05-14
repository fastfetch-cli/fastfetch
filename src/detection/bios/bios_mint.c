#include "bios.h"

#include "common/settings.h"
#include "common/io.h"

#include <mint/sysvars.h>
#include <mint/osbind.h>

const char* ffDetectBios(FFBiosResult* bios) {
    OSHEADER *oh;
    unsigned long oldmode = Super(0L);
    oh    = *(OSHEADER **)_sysbase;
    Super(oldmode);

    ffStrbufSetF(&bios->version, "%2x.%02x", (oh->os_version >> 8), oh->os_version & 0x0ff);
    unsigned long d = (unsigned) oh->os_date;
    // Seems like EmuTOS doesn't agree with TOS on the date format?
    unsigned long y = (d >> 16) & 0x0ffff;
    if ((y >= 0x1980) && (y < 0x2100))
        ffStrbufSetF(&bios->date, "%04x-%02x-%02x", ((d >> 16) & 0x0ffff), ((d >> 8) & 0x0ff), (d & 0x0ff));
    else
        ffStrbufSetF(&bios->date, "%04x-%02x-%02x", (d & 0x0ffff), ((d >> 24) & 0x0ff), ((d >> 16) & 0x0ff));
    ffStrbufSetStatic(&bios->type, "BIOS");

    return NULL;
}
