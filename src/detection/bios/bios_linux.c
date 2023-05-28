#include "bios.h"
#include "common/io/io.h"

#include <stdlib.h>

static bool hostValueSet(const FFstrbuf* value)
{
    const char* str = value->chars;
    const size_t length = value->length;

    return (length > 0) &&
           !(
               ffStrbufStartsWithIgnCaseS(str, length, "To be filled") ||
               ffStrbufStartsWithIgnCaseS(str, length, "To be set") ||
               ffStrbufStartsWithIgnCaseS(str, length, "OEM") ||
               ffStrbufStartsWithIgnCaseS(str, length, "O.E.M.") ||
               ffStrbufIgnCaseCompS(str, length, "None") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "System Product") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "System Product Name") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "System Product Version") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "System Name") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "System Version") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "Default string") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "Undefined") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "Not Specified") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "Not Applicable") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "INVALID") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "Type1ProductConfigId") == 0 ||
               ffStrbufIgnCaseCompS(str, length, "All Series") == 0
           );
}

static void getHostValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffReadFileBuffer(devicesPath, buffer);
    if (hostValueSet(buffer))
        return;

    ffReadFileBuffer(classPath, buffer);
    if (hostValueSet(buffer))
        return;

    ffStrbufClear(buffer);
}

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);
    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    getHostValue("/sys/devices/virtual/dmi/id/bios_date", "/sys/class/dmi/id/bios_date", &bios->biosDate);
    getHostValue("/sys/devices/virtual/dmi/id/bios_release", "/sys/class/dmi/id/bios_release", &bios->biosRelease);
    getHostValue("/sys/devices/virtual/dmi/id/bios_vendor", "/sys/class/dmi/id/bios_vendor", &bios->biosVendor);
    getHostValue("/sys/devices/virtual/dmi/id/bios_version", "/sys/class/dmi/id/bios_version", &bios->biosVersion);
}
