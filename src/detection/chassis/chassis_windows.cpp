extern "C" {
#include "chassis.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"
}
#include "util/windows/unicode.hpp"
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
        if (auto vtProp = record.get(L"ChassisTypes"))
        {
            auto [arr, len] = vtProp.get<std::pair<const int32_t*, uint32_t>>();
            if(len == 0)
                return "ChassisTypes contain no data";
            for (uint32_t i = 0; i < len; ++i)
            {
                if (i > 0)
                    ffStrbufAppendS(&result->type, ", ");
                ffStrbufAppendS(&result->type, ffChassisTypeToString((uint32_t) arr[i]));
            }
        }
        else
            return "Get ChassisTypes failed";

        if (auto vtProp = record.get(L"Version"))
        {
            ffStrbufSetWSV(&result->version, vtProp.get<std::wstring_view>());
            ffCleanUpSmbiosValue(&result->version);
        }

        if (auto vtProp = record.get(L"Manufacturer"))
        {
            ffStrbufSetWSV(&result->vendor, vtProp.get<std::wstring_view>());
            ffCleanUpSmbiosValue(&result->vendor);
        }

        return NULL;
    }
    return "No WMI result returned";
}

extern "C"
const char* ffDetectChassis(FFChassisResult* result, FFChassisOptions* options)
{
    if (options->useWmi)
        return detectWithWmi(result);
    return detectWithRegistry(result);
}
