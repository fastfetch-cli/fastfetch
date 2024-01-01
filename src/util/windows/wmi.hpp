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
    IWbemClassObject* obj;

    explicit FFWmiRecord(IEnumWbemClassObject* pEnumerator): obj(nullptr) {
        if(!pEnumerator) return;

        ULONG ret;
        bool ok = SUCCEEDED(pEnumerator->Next(instance.config.general.wmiTimeout, 1, &obj, &ret)) && ret;
        if(!ok) obj = nullptr;
    }
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
        FFWmiRecord result(pEnumerator);
        return result;
    }
};

#else
    // Win32 COM headers requires C++ compiler
    #error Must be included in C++ source file
#endif //__cplusplus
