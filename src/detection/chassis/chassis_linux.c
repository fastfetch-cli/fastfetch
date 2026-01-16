#include "chassis.h"
#include "common/io.h"
#include "common/smbiosHelper.h"

#include <ctype.h>

const char* ffDetectChassis(FFChassisResult* result)
{
    if (ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_type", "/sys/class/dmi/id/chassis_type", &result->type))
    {
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_serial", "/sys/class/dmi/id/chassis_serial", &result->serial);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_vendor", "/sys/class/dmi/id/chassis_vendor", &result->vendor);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_version", "/sys/class/dmi/id/chassis_version", &result->version);

        if(result->type.length)
        {
            const char* typeStr = ffChassisTypeToString((uint32_t) ffStrbufToUInt(&result->type, 9999));
            if(typeStr)
                ffStrbufSetS(&result->type, typeStr);
        }
    }

    else if(ffReadFileBuffer("/proc/device-tree/chassis-type", &result->type) && result->type.length > 0)
    {
        ffStrbufTrimRight(&result->type, '\0');
        result->type.chars[0] = (char) toupper(result->type.chars[0]);
    }
    return NULL;
}
