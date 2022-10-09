extern "C" {
#include "bios.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);

    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Name, ReleaseDate, Version, Manufacturer FROM Win32_BIOS", &bios->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if(uReturn == 0)
    {
        ffStrbufInitS(&bios->error, "No Wmi result returned");
        pEnumerator->Release();
        return;
    }

    ffGetWmiObjString(pclsObj, L"Name", &bios->biosRelease);
    ffGetWmiObjString(pclsObj, L"ReleaseDate", &bios->biosDate);
    ffGetWmiObjString(pclsObj, L"Version", &bios->biosVersion);
    ffGetWmiObjString(pclsObj, L"Manufacturer", &bios->biosVendor);

    pclsObj->Release();
    pEnumerator->Release();
}
