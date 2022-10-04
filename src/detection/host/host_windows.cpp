extern "C" {
#include "host.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);
    ffStrbufInit(&host->boardName);
    ffStrbufInit(&host->boardVendor);
    ffStrbufInit(&host->boardVersion);
    ffStrbufInit(&host->chassisType);
    ffStrbufInit(&host->chassisVendor);
    ffStrbufInit(&host->chassisVersion);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Name, Version, SKUNumber, Vendor FROM Win32_ComputerSystemProduct", &host->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if(uReturn == 0)
    {
        ffStrbufInitS(&host->error, "No Wmi result returned");
        pEnumerator->Release();
        return;
    }

    ffGetWmiObjValue(pclsObj, L"Name", &host->productName);
    ffGetWmiObjValue(pclsObj, L"Version", &host->productVersion);
    ffGetWmiObjValue(pclsObj, L"SKUNumber", &host->productSku);
    ffGetWmiObjValue(pclsObj, L"Vendor", &host->sysVendor);

    pclsObj->Release();
    pEnumerator->Release();
}
