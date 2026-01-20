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
                ffStrbufSetStatic(&result->type, typeStr);
        }
    }
    else
    {
        // Available on Asahi Linux
        uint32_t chassisType = 0;
        if (ffReadFileData("/sys/firmware/devicetree/base/smbios/smbios/chassis/chassis-type", sizeof(chassisType), &chassisType)) // big endian
        {
            chassisType = __builtin_bswap32(chassisType);
            const char* typeStr = ffChassisTypeToString(chassisType);
            if(typeStr)
                ffStrbufSetStatic(&result->type, typeStr);

            if(ffReadFileBuffer("/sys/firmware/devicetree/base/smbios/smbios/chassis/manufacturer", &result->vendor) && result->vendor.length > 0)
                ffStrbufTrimRight(&result->vendor, '\0');
        }
        else if(ffReadFileBuffer("/sys/firmware/devicetree/base/chassis-type", &result->type) && result->type.length > 0)
        {
            ffStrbufTrimRight(&result->type, '\0');
            result->type.chars[0] = (char) toupper(result->type.chars[0]);
        }
    }
    return NULL;
}
