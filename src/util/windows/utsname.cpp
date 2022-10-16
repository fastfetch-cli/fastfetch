#include "util/windows/wmi.hpp"
extern "C" {
    #include "utsname.h"
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>

int uname(struct utsname *name)
{
    memset(name, 0, sizeof(*name));

    strncpy(name->sysname, "Windows_NT", UTSNAME_MAXLENGTH);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Version, CSName, OSArchitecture FROM Win32_OperatingSystem", nullptr);
    if(!pEnumerator)
        return -1;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        pEnumerator->Release();
        return -1;
    }

    FFstrbuf value;
    ffStrbufInit(&value);
    ffGetWmiObjString(pclsObj, L"Version", &value);
    strncpy(name->release, value.chars, UTSNAME_MAXLENGTH);

    ffStrbufClear(&value);
    ffGetWmiObjString(pclsObj, L"CSName", &value);
    strncpy(name->nodename, value.chars, UTSNAME_MAXLENGTH);

    ffStrbufClear(&value);
    ffGetWmiObjString(pclsObj, L"OSArchitecture", &value);
    strncpy(name->machine, value.chars, UTSNAME_MAXLENGTH);

    ffStrbufDestroy(&value);
    pclsObj->Release();
    pEnumerator->Release();

    return 0;
}
