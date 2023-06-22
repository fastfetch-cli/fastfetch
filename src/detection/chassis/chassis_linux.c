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
    getSmbiosValue("/sys/devices/virtual/dmi/id/chassis_type", "/sys/class/dmi/id/chassis_type", &result->type);
    getSmbiosValue("/sys/devices/virtual/dmi/id/chassis_vendor", "/sys/class/dmi/id/chassis_vendor", &result->vendor);
    getSmbiosValue("/sys/devices/virtual/dmi/id/chassis_version", "/sys/class/dmi/id/chassis_version", &result->version);

    if(result->type.length)
    {
        const char* typeStr = ffChassisTypeToString(ffStrbufToUInt16(&result->type, 9999));
        if(typeStr)
            ffStrbufSetS(&result->type, typeStr);
    }
    return NULL;
}
