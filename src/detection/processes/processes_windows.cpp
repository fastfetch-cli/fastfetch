extern "C" {
#include "processes.h"
}
#include "util/windows/wmi.hpp"

uint32_t ffDetectProcesses(FFinstance* instance, FFstrbuf* error)
{
    FF_UNUSED(instance);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT NumberOfProcesses FROM Win32_OperatingSystem", error);

    if(!pEnumerator)
        return 0;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        ffStrbufAppendS(error, "No Wmi result returned");
        pEnumerator->Release();
        return 0;
    }

    int64_t result = 0;
    ffGetWmiObjInteger(pclsObj, L"NumberOfProcesses", &result);
    pclsObj->Release();
    pEnumerator->Release();
    return (uint32_t)result;
}
