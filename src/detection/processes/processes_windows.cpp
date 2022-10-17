extern "C" {
#include "processes.h"
}
#include "util/windows/wmi.hpp"

uint32_t ffDetectProcesses(FFinstance* instance, FFstrbuf* error)
{
    FF_UNUSED(instance);

    FFWmiQuery query(L"SELECT NumberOfProcesses FROM Win32_OperatingSystem", error);
    if(!query)
        return 0;

    if(FFWmiRecord record = query.next())
    {
        uint64_t result = 0;
        record.getUnsigned(L"NumberOfProcesses", &result);
        return (uint32_t)result;
    }
    else
    {
        ffStrbufAppendS(error, "No Wmi result returned");
        return 0;
    }
}
