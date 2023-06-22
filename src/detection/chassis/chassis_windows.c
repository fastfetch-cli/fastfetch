#include "chassis.h"
#include "util/windows/registry.h"

const char* ffDetectChassis(FFChassisResult* result)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL)) // SMBIOS
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    uint32_t chassisType = 0;
    if(!ffRegReadUint(hKey, L"EnclosureType", &chassisType, NULL))
        return "\"HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS\\EnclosureType\" doesn't exist";

    const char* chassisTypeStr = ffChassisTypeToString(chassisType);
    if(!chassisTypeStr)
        return "Unknown chassis type";

    ffStrbufAppendS(&result->chassisType, chassisTypeStr);
    return NULL;
}
