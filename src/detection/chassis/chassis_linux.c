#include "chassis.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

const char* ffDetectChassis(FFChassisResult* result, FF_MAYBE_UNUSED FFChassisOptions* options)
{
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_type", "/sys/class/dmi/id/chassis_type", &result->type);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_vendor", "/sys/class/dmi/id/chassis_vendor", &result->vendor);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/chassis_version", "/sys/class/dmi/id/chassis_version", &result->version);

    if(result->type.length)
    {
        const char* typeStr = ffChassisTypeToString((uint32_t) ffStrbufToUInt(&result->type, 9999));
        if(typeStr)
            ffStrbufSetS(&result->type, typeStr);
    }
    return NULL;
}
