extern "C" {
#include "font.h"
#include "common/font.h"
}
#include "util/windows/wmi.hpp"

#include <wchar.h>

extern "C"
void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    wchar_t query[256] = {};
    swprintf(query, 256, L"SELECT IconTitleFaceName, IconTitleSize FROM Win32_Desktop WHERE Name LIKE '%%\\\\%s'", instance->state.passwd->pw_name);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(query, &result->error);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if(uReturn == 0)
    {
        ffStrbufInitS(&result->error, "No WMI result returned");
        pEnumerator->Release();
        return;
    }

    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffGetWmiObjString(pclsObj, L"IconTitleFaceName", &fontName);

    uint64_t fontSize;
    ffGetWmiObjUnsigned(pclsObj, L"IconTitleSize", &fontSize);

    ffStrbufAppendF(&result->fonts[0], "%*s (%upt)", fontName.length, fontName.chars, (unsigned)fontSize);

    ffStrbufDestroy(&fontName);

    pclsObj->Release();
    pEnumerator->Release();
}
