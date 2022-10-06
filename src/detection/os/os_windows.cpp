extern "C" {
#include "os.h"
}
#include "util/windows/wmi.hpp"

extern "C"
void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance)
{
    ffStrbufInit(&os->name);
    ffStrbufInit(&os->prettyName);
    ffStrbufInit(&os->id);
    ffStrbufInit(&os->idLike);
    ffStrbufInit(&os->variant);
    ffStrbufInit(&os->variantID);
    ffStrbufInit(&os->version);
    ffStrbufInit(&os->versionID);
    ffStrbufInit(&os->codename);
    ffStrbufInit(&os->buildID);
    ffStrbufInit(&os->systemName);
    ffStrbufInit(&os->architecture);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Caption, Version, BuildNumber, OSArchitecture FROM Win32_OperatingSystem", nullptr);

    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        pEnumerator->Release();
        return;
    }

    ffGetWmiObjValue(pclsObj, L"Caption", &os->variant); // Microsoft Windows 11 家庭中文版
    if(ffStrbufStartsWithS(&os->variant, "Microsoft Windows "))
    {
        ffStrbufAppendS(&os->name, "Microsoft Windows");
        ffStrbufAppendS(&os->prettyName, "Windows");

        ffStrbufSubstrAfter(&os->variant, strlen("Microsoft Windows ") - 1); // 11 家庭中文版
        uint32_t index = ffStrbufFirstIndexC(&os->variant, ' ');
        ffStrbufAppendNS(&os->version, index, os->variant.chars);
        ffStrbufSubstrAfter(&os->variant, index);
    }
    else
    {
        // Unknown Windows name, please report this
        ffStrbufAppend(&os->name, &os->variant);
        ffStrbufClear(&os->variant);
    }

    #ifdef __CYGWIN__
        ffStrbufAppendS(&os->id, "MSYS2");
    #else
        // Enable this after we have Windows logo support
        ffStrbufAppendF(&os->id, "Windows %*s", &os->version.length, &os->version.chars);
    #endif

    ffGetWmiObjValue(pclsObj, L"BuildNumber", &os->buildID);
    ffGetWmiObjValue(pclsObj, L"OSArchitecture", &os->architecture);

    #ifdef __CYGWIN__
        ffStrbufSetS(&os->systemName, instance->state.utsname.sysname);
    #else
        ffStrbufSetS(&os->systemName, "Windows");
    #endif

    pclsObj->Release();
    pEnumerator->Release();
}
