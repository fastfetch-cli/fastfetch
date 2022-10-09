extern "C" {
#include "cpuUsage.h"
}
#include "util/windows/wmi.hpp"

extern "C" const char* ffGetCpuUsageResultNoWait(double* result)
{
    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT LoadPercentage FROM Win32_Processor", nullptr);
    if(!pEnumerator)
        return "Query WMI service failed";

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        pEnumerator->Release();
        return "No WMI result returned";
    }

    ffGetWmiObjReal(pclsObj, L"LoadPercentage", result);

    pclsObj->Release();
    pEnumerator->Release();
    return NULL;
}
