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
        uint64_t driveType;
        ffGetWmiObjUnsigned(pclsObj, L"DriveType", &driveType);
        if(driveType == 2 && !instance->config.diskRemovable)
            continue;

        FFDiskResult* folder = (FFDiskResult*)ffListAdd(folders);

        ffStrbufInit(&folder->path);
        ffGetWmiObjString(pclsObj, L"Name", &folder->path);

        uint64_t free;
        ffGetWmiObjUnsigned(pclsObj, L"Size", &folder->total);
        ffGetWmiObjUnsigned(pclsObj, L"FreeSpace", &free);
        folder->used = folder->total - free;

        folder->files = 0; //Unsupported

        ffStrbufInit(&folder->error);
    }

    pclsObj->Release();
    pEnumerator->Release();
    return nullptr;
}

bool ffDiskDetectDiskFolders(FFinstance*, FFlist*)
{
    return false; // Unsupported
}
