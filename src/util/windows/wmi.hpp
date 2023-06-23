#pragma once

#ifndef FF_INCLUDED_util_windows_wmi
#define FF_INCLUDED_util_windows_wmi

#ifdef __cplusplus

extern "C" {
    #include "util/FFstrbuf.h"
}

#include <initguid.h>
#include <Wbemidl.h>
#include <utility>
#include <string_view>
#include <cassert>

enum class FFWmiNamespace {
    CIMV2,
    WMI,
    LAST,
};

enum {
    FF_WMI_QUERY_TIMEOUT = 5000
};

struct FFWmiRecord
{
    IWbemClassObject* obj;

    explicit FFWmiRecord(IEnumWbemClassObject* pEnumerator): obj(nullptr) {
        if(!pEnumerator) return;

        ULONG ret;
        bool ok = SUCCEEDED(pEnumerator->Next(FF_WMI_QUERY_TIMEOUT, 1, &obj, &ret)) && ret;
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

struct FFWmiVariant: VARIANT {
    explicit FFWmiVariant() { VariantInit(this); }
    ~FFWmiVariant() { VariantClear(this); }

    FFWmiVariant(const FFWmiVariant&) = delete;

    // boolean
    explicit operator bool() {
        assert(this->vt == VT_BOOL);
        return this->boolVal != VARIANT_FALSE;
    }

    // signed
    explicit operator int8_t() {
        assert(this->vt == VT_I1);
        return this->cVal;
    }
    explicit operator int16_t() {
        assert(vt == VT_I2);
        return this->iVal;
    }
    explicit operator int32_t() {
        assert(this->vt == VT_I4 || vt == VT_INT);
        return this->intVal;
    }
    explicit operator int64_t() {
        assert(this->vt == VT_I8);
        return this->llVal;
    }

    // unsigned
    explicit operator uint8_t() {
        assert(this->vt == VT_I1);
        return this->bVal;
    }
    explicit operator uint16_t() {
        assert(this->vt == VT_I2);
        return this->uiVal;
    }
    explicit operator uint32_t() {
        assert(this->vt == VT_UI4 || vt == VT_UINT);
        return this->uintVal;
    }
    explicit operator uint64_t() {
        assert(this->vt == VT_UI8);
        return this->ullVal;
    }
    explicit operator float() {
        assert(this->vt == VT_R4);
        return this->fltVal;
    }
    explicit operator double() {
        assert(this->vt == VT_R8);
        return this->dblVal;
    }

    // string
    explicit operator const std::string_view() {
        assert(this->vt == VT_LPSTR);
        return this->pcVal;
    }
    explicit operator const std::wstring_view() {
        assert(this->vt == VT_BSTR || this->vt == VT_LPWSTR);
        return this->bstrVal;
    }

    // array signed
    explicit operator std::pair<const int8_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_I1);
        return std::make_pair((int8_t*)this->parray->pvData, this->parray->cDims);
    }
    explicit operator std::pair<const int16_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_I2);
        return std::make_pair((int16_t*)this->parray->pvData, this->parray->cDims);
    }
    explicit operator std::pair<const int32_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_I4);
        return std::make_pair((int32_t*)this->parray->pvData, this->parray->cDims);
    }
    explicit operator std::pair<const int64_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_I8);
        return std::make_pair((int64_t*)this->parray->pvData, this->parray->cDims);
    }

    // array unsigned
    explicit operator std::pair<const uint8_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_UI1);
        return std::make_pair((uint8_t*)this->parray->pvData, this->parray->cDims);
    }
    explicit operator std::pair<const uint16_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_UI2);
        return std::make_pair((uint16_t*)this->parray->pvData, this->parray->cDims);
    }
    explicit operator std::pair<const uint32_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_UI4);
        return std::make_pair((uint32_t*)this->parray->pvData, this->parray->cDims);
    }
    explicit operator std::pair<const uint64_t*, uint32_t>() {
        assert(this->vt & VT_ARRAY);
        assert((this->vt & ~VT_ARRAY) == VT_UI8);
        return std::make_pair((uint64_t*)this->parray->pvData, this->parray->cDims);
    }
};

#else
    // Win32 COM headers requires C++ compiler
    #error Must be included in C++ source file
#endif //__cplusplus

#endif //FF_INCLUDED_util_windows_wmi
