#pragma once

#ifdef __cplusplus

extern "C" {
    #include "fastfetch.h"
}

#include <initguid.h>
#include <Wbemidl.h>
#include <cassert>

#include "variant.hpp"

enum class FFWmiNamespace {
    CIMV2,
    WMI,
    LAST,
};

struct FFWmiRecord
{
    IWbemClassObject* obj = nullptr;

    explicit FFWmiRecord(IWbemClassObject* obj): obj(obj) {};
    FFWmiRecord(const FFWmiRecord&) = delete;
    FFWmiRecord(FFWmiRecord&& other) { *this = (FFWmiRecord&&)other; }
    ~FFWmiRecord() { if(obj) obj->Release(); }
    explicit operator bool() { return !!obj; }
    FFWmiRecord& operator =(FFWmiRecord&& other) {
        if(obj) obj->Release();
        obj = other.obj;
        other.obj = nullptr;
        return *this;
    }

    bool getString(const wchar_t* key, FFstrbuf* strbuf);
    bool getSigned(const wchar_t* key, int64_t* integer);
    bool getUnsigned(const wchar_t* key, uint64_t* integer);
    bool getReal(const wchar_t* key, double* real);
    FFWmiVariant get(const wchar_t* key) {
        FFWmiVariant result;
        obj->Get(key, 0, &result, nullptr, nullptr);
        return result;
    }
};

struct FFWmiQuery
{
    IWbemServices* pService = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;

    FFWmiQuery(const wchar_t* queryStr, FFstrbuf* error = nullptr, FFWmiNamespace wmiNs = FFWmiNamespace::CIMV2);
    explicit FFWmiQuery(IEnumWbemClassObject* pEnumerator): pEnumerator(pEnumerator) {}
    FFWmiQuery(const FFWmiQuery& other) = delete;
    FFWmiQuery(FFWmiQuery&& other) { *this = (FFWmiQuery&&)other; }
    ~FFWmiQuery() { if(pEnumerator) pEnumerator->Release(); }

    explicit operator bool() { return !!pEnumerator; }
    FFWmiQuery& operator =(FFWmiQuery&& other) {
        if(pEnumerator) pEnumerator->Release();
        pEnumerator = other.pEnumerator;
        other.pEnumerator = nullptr;
        return *this;
    }

    FFWmiRecord next() {
        IWbemClassObject* obj = nullptr;
        ULONG ret;
        pEnumerator->Next(instance.config.general.wmiTimeout, 1, &obj, &ret);

        FFWmiRecord result(obj);
        return result;
    }
};

namespace
{
    // Provide our bstr_t to avoid libstdc++ dependency
    struct bstr_t
    {
        explicit bstr_t(const wchar_t* str) noexcept: _bstr(SysAllocString(str)) {}
        ~bstr_t(void) noexcept { SysFreeString(_bstr); }
        explicit operator const wchar_t*(void) const noexcept { return _bstr; }
        operator BSTR(void) const noexcept { return _bstr; }

        private:
            BSTR _bstr;
    };
}

#else
    // Win32 COM headers requires C++ compiler
    #error Must be included in C++ source file
#endif //__cplusplus
