extern "C" {
#include "gpu.h"
}
#include "util/windows/wmi.hpp"

extern "C"
const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Name, AdapterCompatibility, DriverVersion FROM Win32_VideoController", nullptr);

    if(!pEnumerator)
        return "Query WMI service failed";

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while(SUCCEEDED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) && uReturn != 0)
    {
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);

        ffStrbufInit(&gpu->vendor);
        ffGetWmiObjString(pclsObj, L"AdapterCompatibility", &gpu->vendor);
        if(ffStrbufStartsWithS(&gpu->vendor, "Intel "))
        {
            //Intel returns "Intel Corporation", not sure about AMD
            ffStrbufSetS(&gpu->vendor, "Intel");
        }

        ffStrbufInit(&gpu->name);
        ffGetWmiObjString(pclsObj, L"Name", &gpu->name);

        ffStrbufInit(&gpu->driver);
        ffGetWmiObjString(pclsObj, L"DriverVersion", &gpu->driver);

        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    }

    if(pclsObj) pclsObj->Release();
    pEnumerator->Release();
    return nullptr;
}
