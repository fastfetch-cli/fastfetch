extern "C" {
#include "board.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInit(&board->error);

    ffStrbufInit(&board->boardName);
    ffStrbufInit(&board->boardVendor);
    ffStrbufInit(&board->boardVersion);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Product, Version, Manufacturer FROM Win32_BaseBoard", &board->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if(uReturn == 0)
    {
        ffStrbufInitS(&board->error, "No Wmi result returned");
        pEnumerator->Release();
        return;
    }

    ffGetWmiObjString(pclsObj, L"Product", &board->boardName);
    ffGetWmiObjString(pclsObj, L"Manufacturer", &board->boardVendor);
    ffGetWmiObjString(pclsObj, L"Version", &board->boardVersion);

    pclsObj->Release();
    pEnumerator->Release();
}
