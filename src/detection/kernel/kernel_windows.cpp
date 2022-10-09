extern "C" {
#include "kernel.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectKernel(FFinstance* instance, FFKernelResult* kernel)
{
    FF_UNUSED(instance);

    ffStrbufInit(&kernel->error);

    ffStrbufInitS(&kernel->sysname, "Windows_NT");
    ffStrbufInit(&kernel->release);
    ffStrbufInit(&kernel->version);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Version FROM Win32_OperatingSystem", &kernel->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        ffStrbufInitS(&kernel->error, "No WMI result returned");
        pEnumerator->Release();
        return;
    }

    ffGetWmiObjString(pclsObj, L"Version", &kernel->release);

    pclsObj->Release();
    pEnumerator->Release();
}
