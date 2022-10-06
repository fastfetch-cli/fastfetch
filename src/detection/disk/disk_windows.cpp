extern "C" {
#include "disk.h"
}
#include "util/windows/wmi.hpp"

const char* ffDiskAutodetectFolders(FFinstance* instance, FFlist* folders)
{
    FF_UNUSED(instance);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Name, DriveType, FreeSpace, Size FROM Win32_LogicalDisk", nullptr);

    if(!pEnumerator)
        return "Query WMI service failed";

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while(SUCCEEDED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) && uReturn != 0)
    {
        int64_t driveType;
        ffGetWmiObjInteger(pclsObj, L"DriveType", &driveType);
        if(driveType == 2 && !instance->config.diskRemovable)
            continue;

        FFDiskResult* folder = (FFDiskResult*)ffListAdd(folders);

        ffStrbufInit(&folder->path);
        ffGetWmiObjValue(pclsObj, L"Name", &folder->path);

        ffGetWmiObjInteger(pclsObj, L"Size", &folder->total);

        uint64_t freeSpace;
        ffGetWmiObjInteger(pclsObj, L"FreeSpace", &freeSpace);
        folder->used = folder->total - freeSpace;
    }

    pclsObj->Release();
    pEnumerator->Release();
    return nullptr;
}
