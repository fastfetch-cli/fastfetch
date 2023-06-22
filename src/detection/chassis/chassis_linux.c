#include "chassis.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

static void getSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffReadFileBuffer(devicesPath, buffer);
    if(ffIsSmbiosValueSet(buffer))
        return;

    ffReadFileBuffer(classPath, buffer);
    if(ffIsSmbiosValueSet(buffer))
        return;

    ffStrbufClear(buffer);
}

const char* ffDetectChassis(FFChassisResult* result)
{
    getSmbiosValue("/sys/devices/virtual/dmi/id/chassis_type", "/sys/class/dmi/id/chassis_type", &result->chassisType);
    getSmbiosValue("/sys/devices/virtual/dmi/id/chassis_vendor", "/sys/class/dmi/id/chassis_vendor", &result->chassisVendor);
    getSmbiosValue("/sys/devices/virtual/dmi/id/chassis_version", "/sys/class/dmi/id/chassis_version", &result->chassisVersion);

    if(result->chassisType.length)
    {
        const char* chassisTypeStr = ffChassisTypeToString(ffStrbufToUInt16(&result->chassisType, 9999));
        if(chassisTypeStr)
            ffStrbufSetS(&result->chassisType, chassisTypeStr);
    }
    return NULL;
}
