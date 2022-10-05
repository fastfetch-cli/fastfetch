#pragma once

#ifndef FF_INCLUDED_util_windows_wmi
#define FF_INCLUDED_util_windows_wmi

#ifdef __cplusplus

extern "C" {
    #include "util/FFstrbuf.h"
}

#include <Wbemidl.h>

//<comdef.h> is not usable in MSYS, so provide our simple bstr_t implementation
struct bstr_t
{
    explicit bstr_t(const wchar_t* str) noexcept: _bstr(SysAllocString(str)) {}

    ~bstr_t() noexcept { SysFreeString(_bstr); }

    explicit operator const wchar_t*() const noexcept {
        return _bstr;
    }

    operator BSTR() const noexcept {
        return _bstr;
    }

private:
    BSTR _bstr;
};

void ffBstrToStrbuf(BSTR bstr, FFstrbuf* strbuf);

IEnumWbemClassObject* ffQueryWmi(const wchar_t* queryStr, FFstrbuf* error);
bool ffGetWmiObjValue(IWbemClassObject* obj, const wchar_t* key, FFstrbuf* strbuf);
bool ffGetWmiObjInteger(IWbemClassObject* obj, const wchar_t* key, int64_t* result);

#else
    // Win32 COM headers requires C++ compiler
    #error Must be included in C++ source file
#endif //__cplusplus

#endif //FF_INCLUDED_util_windows_wmi
