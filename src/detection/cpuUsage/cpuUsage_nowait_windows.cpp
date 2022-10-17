extern "C" {
#include "cpuUsage.h"
}
#include "util/windows/wmi.hpp"

extern "C" const char* ffGetCpuUsageResultNoWait(double* result)
{
    FFWmiQuery query(L"SELECT LoadPercentage FROM Win32_Processor");
    if(!query)
        return "Query WMI service failed";

    if(FFWmiRecord record = query.next())
        record.getReal(L"LoadPercentage", result);
    else
        return "No WMI result returned";

    return nullptr;
}
