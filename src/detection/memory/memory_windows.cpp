extern "C" {
#include "memory.h"
}
#include "util/windows/wmi.hpp"

void detectRam(FFMemoryStorage* ram)
{
    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT TotalVisibleMemorySize, FreePhysicalMemory FROM Win32_OperatingSystem", &ram->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        ffStrbufInitS(&ram->error, "No WMI result returned");
        pEnumerator->Release();
        return;
    }

    //KB
    ffGetWmiObjUnsigned(pclsObj, L"TotalVisibleMemorySize", &ram->bytesTotal);
    uint64_t bytesFree;
    ffGetWmiObjUnsigned(pclsObj, L"FreePhysicalMemory", &bytesFree);
    ram->bytesUsed = ram->bytesTotal - bytesFree;

    pclsObj->Release();
    pEnumerator->Release();

    ram->bytesTotal *= 1024;
    ram->bytesUsed *= 1024;
}

void detectSwap(FFMemoryStorage* swap)
{
    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT AllocatedBaseSize, CurrentUsage FROM Win32_PageFileUsage", &swap->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        ffStrbufInitS(&swap->error, "No WMI result returned");
        pEnumerator->Release();
        return;
    }

    //MB
    ffGetWmiObjUnsigned(pclsObj, L"AllocatedBaseSize", &swap->bytesTotal);
    ffGetWmiObjUnsigned(pclsObj, L"CurrentUsage", &swap->bytesUsed);

    pclsObj->Release();
    pEnumerator->Release();

    swap->bytesTotal *= 1024 * 1024;
    swap->bytesUsed *= 1024 * 1024;
}

extern "C"
void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    detectRam(&memory->ram);
    detectSwap(&memory->swap);
}
