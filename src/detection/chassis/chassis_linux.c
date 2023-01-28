#include "chassis.h"
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

void ffDetectChassis(FFChassisResult* result)
{
    ffStrbufInit(&result->error);

    ffStrbufInit(&result->chassisType);
    getHostValue("/sys/devices/virtual/dmi/id/chassis_type", "/sys/class/dmi/id/chassis_type", &result->chassisType);

    ffStrbufInit(&result->chassisVendor);
    getHostValue("/sys/devices/virtual/dmi/id/chassis_vendor", "/sys/class/dmi/id/chassis_vendor", &result->chassisVendor);

    ffStrbufInit(&result->chassisVersion);
    getHostValue("/sys/devices/virtual/dmi/id/chassis_version", "/sys/class/dmi/id/chassis_version", &result->chassisVersion);
}
