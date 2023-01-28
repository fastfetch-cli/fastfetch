#include "bios.h"
#include "common/io/io.h"

#include <stdlib.h>

static bool hostValueSet(FFstrbuf* value)
{
    return
        value->length > 0 &&
        ffStrbufStartsWithIgnCaseS(value, "To be filled") != true &&
        ffStrbufStartsWithIgnCaseS(value, "To be set") != true &&
        ffStrbufStartsWithIgnCaseS(value, "OEM") != true &&
        ffStrbufStartsWithIgnCaseS(value, "O.E.M.") != true &&
        ffStrbufIgnCaseCompS(value, "None") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "Default string") != 0 &&
        ffStrbufIgnCaseCompS(value, "Undefined") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Specified") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Applicable") != 0 &&
        ffStrbufIgnCaseCompS(value, "INVALID") != 0 &&
        ffStrbufIgnCaseCompS(value, "Type1ProductConfigId") != 0 &&
        ffStrbufIgnCaseCompS(value, "All Series") != 0
    ;
}

static void getHostValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffReadFileBuffer(devicesPath, buffer);
    if(hostValueSet(buffer))
        return;

    ffReadFileBuffer(classPath, buffer);
    if(hostValueSet(buffer))
        return;

    ffStrbufClear(buffer);
}

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);

    ffStrbufInit(&bios->biosDate);
    getHostValue("/sys/devices/virtual/dmi/id/bios_date", "/sys/class/dmi/id/bios_date", &bios->biosDate);

    ffStrbufInit(&bios->biosRelease);
    getHostValue("/sys/devices/virtual/dmi/id/bios_release", "/sys/class/dmi/id/bios_release", &bios->biosRelease);

    ffStrbufInit(&bios->biosVendor);
    getHostValue("/sys/devices/virtual/dmi/id/bios_vendor", "/sys/class/dmi/id/bios_vendor", &bios->biosVendor);

    ffStrbufInit(&bios->biosVersion);
    getHostValue("/sys/devices/virtual/dmi/id/bios_version", "/sys/class/dmi/id/bios_version", &bios->biosVersion);
}
