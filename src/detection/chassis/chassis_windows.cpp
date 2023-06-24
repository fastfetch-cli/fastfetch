extern "C" {
#include "chassis.h"
#include "util/windows/registry.h"
}
#include "util/windows/wmi.hpp"

static const char* detectWithRegistry(FFChassisResult* result)
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

FF_MAYBE_UNUSED static const char* detectWithWmi(FFChassisResult* result)
{
    FFWmiQuery query(L"SELECT Version, ChassisTypes, Manufacturer FROM Win32_SystemEnclosure");
    if (!query)
        return "Query WMI service failed";

    if (FFWmiRecord record = query.next())
    {
        FFWmiVariant vtProp;
        if(FAILED(record.obj->Get(L"ChassisTypes", 0, &vtProp, nullptr, nullptr)))
            return "Get ChassisTypes failed";

        auto [arr, len] = (std::pair<const int32_t*, uint32_t>) vtProp;

        if(len == 0)
            return "ChassisTypes contain no data failed";

        ffStrbufAppendS(&result->type, ffChassisTypeToString((uint32_t) *arr));

        record.getString(L"Version", &result->version);
        record.getString(L"Manufacturer", &result->vendor);
        return NULL;
    }
    return "No WMI result returned";
}

extern "C"
const char* ffDetectChassis(FFChassisResult* result)
{
    return detectWithRegistry(result);
    // TODO: if (instance.config.allowSlowOperations) detectWithWmi(result);
}
