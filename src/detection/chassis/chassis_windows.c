#include "chassis.h"
#include "util/windows/registry.h"

const char* ffDetectChassis(FFChassisResult* result)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL)) // SMBIOS
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    uint32_t type = 0;
    if(!ffRegReadUint(hKey, L"EnclosureType", &type, NULL))
        return "\"HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS\\EnclosureType\" doesn't exist";

    const char* typeStr = ffChassisTypeToString(type);
    if(!typeStr)
        return "Unknown chassis type";

    ffStrbufAppendS(&result->type, typeStr);
    return NULL;
}
