#include <oaidl.h>
#include <type_traits>
#include <utility>
#include <string_view>

struct FFWmiVariant: VARIANT
{
    explicit FFWmiVariant() { VariantInit(this); }
    ~FFWmiVariant() { VariantClear(this); }

    FFWmiVariant(const FFWmiVariant&) = delete;
    FFWmiVariant(FFWmiVariant&&); // don't define it to enforce NRVO optimization

    bool hasValue() {
        return this->vt != VT_EMPTY;
    }

    explicit operator bool() {
        return this->hasValue();
    }

    template <typename T> T get()
    {
        // boolean
        if constexpr (std::is_same_v<T, bool>) {
            assert(this->vt == VT_BOOL);
            return this->boolVal != VARIANT_FALSE;
        }

        // signed
        else if constexpr (std::is_same_v<T, int8_t>) {
            assert(this->vt == VT_I1);
            return this->cVal;
        }
        else if constexpr (std::is_same_v<T, int16_t>) {
            assert(vt == VT_I2);
            return this->iVal;
        }
        else if constexpr (std::is_same_v<T, int32_t>) {
            assert(this->vt == VT_I4 || vt == VT_INT);
            return this->intVal;
        }
        else if constexpr (std::is_same_v<T, int64_t>) {
            assert(this->vt == VT_I8);
            return this->llVal;
        }

        // unsigned
        else if constexpr (std::is_same_v<T, uint8_t>) {
            assert(this->vt == VT_UI1);
            return this->bVal;
        }
        else if constexpr (std::is_same_v<T, uint16_t>) {
            assert(this->vt == VT_UI2);
            return this->uiVal;
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            assert(this->vt == VT_UI4 || vt == VT_UINT);
            return this->uintVal;
        }
        else if constexpr (std::is_same_v<T, uint64_t>) {
            assert(this->vt == VT_UI8);
            return this->ullVal;
        }

        // decimal
        else if constexpr (std::is_same_v<T, float>) {
            assert(this->vt == VT_R4);
            return this->fltVal;
        }
        else if constexpr (std::is_same_v<T, double>) {
            assert(this->vt == VT_R8);
            return this->dblVal;
        }

        // string
        else if constexpr (std::is_same_v<T, std::string_view>) {
            assert(this->vt == VT_LPSTR);
            return this->pcVal;
        }
        else if constexpr (std::is_same_v<T, std::wstring_view>) {
            assert(this->vt == VT_BSTR || this->vt == VT_LPWSTR);
            if (this->vt == VT_LPWSTR)
                return this->bstrVal;
            else
                return { this->bstrVal, SysStringLen(this->bstrVal) };
        }

        // array signed
        else if constexpr (std::is_same_v<T, std::pair<const int8_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_I1);
            return std::make_pair((int8_t*)this->parray->pvData, this->parray->cDims);
        }
        else if constexpr (std::is_same_v<T, std::pair<const int16_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_I2);
            return std::make_pair((int16_t*)this->parray->pvData, this->parray->cDims);
        }
        else if constexpr (std::is_same_v<T, std::pair<const int32_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_I4);
            return std::make_pair((int32_t*)this->parray->pvData, this->parray->cDims);
        }
        else if constexpr (std::is_same_v<T, std::pair<const int64_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_I8);
            return std::make_pair((int64_t*)this->parray->pvData, this->parray->cDims);
        }

        // array unsigned
        else if constexpr (std::is_same_v<T, std::pair<const uint8_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_UI1);
            return std::make_pair((uint8_t*)this->parray->pvData, this->parray->cDims);
        }
        else if constexpr (std::is_same_v<T, std::pair<const uint16_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_UI2);
            return std::make_pair((uint16_t*)this->parray->pvData, this->parray->cDims);
        }
        else if constexpr (std::is_same_v<T, std::pair<const uint32_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_UI4);
            return std::make_pair((uint32_t*)this->parray->pvData, this->parray->cDims);
        }
        else if constexpr (std::is_same_v<T, std::pair<const uint64_t*, uint32_t>>) {
            assert(this->vt & VT_ARRAY);
            assert((this->vt & ~VT_ARRAY) == VT_UI8);
            return std::make_pair((uint64_t*)this->parray->pvData, this->parray->cDims);
        }
        else {
            assert(false && "unsupported type");
            __builtin_unreachable();
        }
    }
};
