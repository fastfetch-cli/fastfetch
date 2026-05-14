#include "bios.h"

#include "common/settings.h"
#include "common/io.h"

#include <mint/sysvars.h>

const char* ffDetectBios(FFBiosResult* bios) {
    OSHEADER *oh = (OSHEADER *)_sysbase;

    ffStrbufSetF(&bios->version, "%d", oh->os_version);
    unsigned long d = (unsigned) oh->os_date;
    ffStrbufSetF(&bios->date, "%04d-%02d-%02d", (d & 0x0ffff), ((d >> 24) & 0x0ff), ((d >> 16) & 0x0ff));
    ffStrbufSetStatic(&bios->type, "BIOS");

    return NULL;
}
