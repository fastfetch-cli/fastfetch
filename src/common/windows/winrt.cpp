#include "winrt.hpp"

#if FF_HAVE_WINRT

    #include <wchar.h>

// A COM vtable is a flat array of function pointers: IUnknown's three methods, IInspectable's three,
// then the interface's own methods in declaration order. The pair below writes that array by hand
// instead of deriving from winrt::impl::abi<...>.
//
// Deriving is the idiomatic choice, and it would let the compiler check every slot -- but it cannot
// be used here. A class with virtual functions makes clang emit the vtable of its abstract base, and
// each pure slot of that table refers to libc++abi's __cxa_pure_virtual. fastfetch is linked by the C
// compiler and therefore has neither libc++ nor libc++abi, so that reference would be the only
// unresolved symbol in the program. A hand-written table has no virtual functions at all, which
// keeps this file as free of the C++ runtime as `media_windows.cpp` is.
namespace {

using WinrtIterableAbi = winrt::impl::abi<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>::type;

struct FFWinrtHstringIterable;
struct FFWinrtHstringIterator;

struct FFWinrtHstringIterableVtbl {
    std::int32_t (STDMETHODCALLTYPE* QueryInterface)(FFWinrtHstringIterable* self, GUID const& riid, void** result);
    std::uint32_t (STDMETHODCALLTYPE* AddRef)(FFWinrtHstringIterable* self);
    std::uint32_t (STDMETHODCALLTYPE* Release)(FFWinrtHstringIterable* self);
    std::int32_t (STDMETHODCALLTYPE* GetIids)(FFWinrtHstringIterable* self, std::uint32_t* count, GUID** iids);
    std::int32_t (STDMETHODCALLTYPE* GetRuntimeClassName)(FFWinrtHstringIterable* self, void** name);
    std::int32_t (STDMETHODCALLTYPE* GetTrustLevel)(FFWinrtHstringIterable* self, std::int32_t* level);
    std::int32_t (STDMETHODCALLTYPE* First)(FFWinrtHstringIterable* self, void** result);
};

struct FFWinrtHstringIteratorVtbl {
    std::int32_t (STDMETHODCALLTYPE* QueryInterface)(FFWinrtHstringIterator* self, GUID const& riid, void** result);
    std::uint32_t (STDMETHODCALLTYPE* AddRef)(FFWinrtHstringIterator* self);
    std::uint32_t (STDMETHODCALLTYPE* Release)(FFWinrtHstringIterator* self);
    std::int32_t (STDMETHODCALLTYPE* GetIids)(FFWinrtHstringIterator* self, std::uint32_t* count, GUID** iids);
    std::int32_t (STDMETHODCALLTYPE* GetRuntimeClassName)(FFWinrtHstringIterator* self, void** name);
    std::int32_t (STDMETHODCALLTYPE* GetTrustLevel)(FFWinrtHstringIterator* self, std::int32_t* level);
    std::int32_t (STDMETHODCALLTYPE* get_Current)(FFWinrtHstringIterator* self, HSTRING* value);
    std::int32_t (STDMETHODCALLTYPE* get_HasCurrent)(FFWinrtHstringIterator* self, bool* value);
    std::int32_t (STDMETHODCALLTYPE* MoveNext)(FFWinrtHstringIterator* self, bool* value);
    std::int32_t (STDMETHODCALLTYPE* GetMany)(FFWinrtHstringIterator* self, std::uint32_t capacity, HSTRING* values, std::uint32_t* actual);
};

// Everything the pair allocates comes from malloc(), because the placement is driven by the C
// allocator rather than by `new` (see the note above).
struct FFWinrtHstringIterable {
    const FFWinrtHstringIterableVtbl* lpVtbl;
    std::uint32_t refCount;
    FFlist strings; // HSTRING
};

struct FFWinrtHstringIterator {
    const FFWinrtHstringIteratorVtbl* lpVtbl;
    std::uint32_t refCount;
    FFWinrtHstringIterable* owner;
    std::uint32_t index;
};

// ---------------------------------------------------------------------------------------------
// IIterable<HSTRING>
// ---------------------------------------------------------------------------------------------

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterable_QueryInterface(FFWinrtHstringIterable* self, GUID const& riid, void** result) {
    if (!result) {
        return E_POINTER;
    }
    *result = nullptr;

    if (IsEqualGUID(riid, winrt::guid_of<winrt::Windows::Foundation::IUnknown>()) ||
        IsEqualGUID(riid, winrt::guid_of<winrt::Windows::Foundation::IInspectable>()) ||
        IsEqualGUID(riid, winrt::guid_of<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>())) {
        *result = self;
        self->lpVtbl->AddRef(self);
        return S_OK;
    }

    return E_NOINTERFACE;
}

std::uint32_t STDMETHODCALLTYPE ffWinrtHstringIterable_AddRef(FFWinrtHstringIterable* self) {
    return ++self->refCount;
}

std::uint32_t STDMETHODCALLTYPE ffWinrtHstringIterable_Release(FFWinrtHstringIterable* self) {
    std::uint32_t remaining = --self->refCount;
    if (remaining == 0) {
        FF_LIST_FOR_EACH(HSTRING, string, self->strings) {
            WindowsDeleteString(*string);
        }
        ffListDestroy(&self->strings);
        free(self);
    }
    return remaining;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterable_GetIids(FFWinrtHstringIterable* self, std::uint32_t* count, GUID** iids) {
    (void) self;
    *count = 0;
    *iids = nullptr;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterable_GetRuntimeClassName(FFWinrtHstringIterable* self, void** name) {
    (void) self;
    *name = nullptr;
    return S_OK;
}

// `level` is the ABI's 32-bit TrustLevel enum; BaseTrust is 0.
std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterable_GetTrustLevel(FFWinrtHstringIterable* self, std::int32_t* level) {
    (void) self;
    *level = 0;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterable_First(FFWinrtHstringIterable* self, void** result) {
    if (!result) {
        return E_POINTER;
    }
    *result = nullptr;

    auto* iterator = (FFWinrtHstringIterator*) malloc(sizeof(FFWinrtHstringIterator));
    if (!iterator) {
        return E_OUTOFMEMORY;
    }

    iterator->lpVtbl = nullptr; // assigned below, once the table is declared
    iterator->refCount = 1;
    iterator->owner = self;
    iterator->index = 0;

    // The iterator outlives the call that created it, so it holds a reference of its own.
    self->lpVtbl->AddRef(self);

    extern const FFWinrtHstringIteratorVtbl ffWinrtHstringIteratorVtbl;
    iterator->lpVtbl = &ffWinrtHstringIteratorVtbl;

    *result = iterator;
    return S_OK;
}

const FFWinrtHstringIterableVtbl ffWinrtHstringIterableVtbl = {
    ffWinrtHstringIterable_QueryInterface,
    ffWinrtHstringIterable_AddRef,
    ffWinrtHstringIterable_Release,
    ffWinrtHstringIterable_GetIids,
    ffWinrtHstringIterable_GetRuntimeClassName,
    ffWinrtHstringIterable_GetTrustLevel,
    ffWinrtHstringIterable_First,
};

// ---------------------------------------------------------------------------------------------
// IIterator<HSTRING>
// ---------------------------------------------------------------------------------------------

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_QueryInterface(FFWinrtHstringIterator* self, GUID const& riid, void** result) {
    if (!result) {
        return E_POINTER;
    }
    *result = nullptr;

    if (IsEqualGUID(riid, winrt::guid_of<winrt::Windows::Foundation::IUnknown>()) ||
        IsEqualGUID(riid, winrt::guid_of<winrt::Windows::Foundation::IInspectable>()) ||
        IsEqualGUID(riid, winrt::guid_of<winrt::Windows::Foundation::Collections::IIterator<winrt::hstring>>())) {
        *result = self;
        self->lpVtbl->AddRef(self);
        return S_OK;
    }

    return E_NOINTERFACE;
}

std::uint32_t STDMETHODCALLTYPE ffWinrtHstringIterator_AddRef(FFWinrtHstringIterator* self) {
    return ++self->refCount;
}

std::uint32_t STDMETHODCALLTYPE ffWinrtHstringIterator_Release(FFWinrtHstringIterator* self) {
    std::uint32_t remaining = --self->refCount;
    if (remaining == 0) {
        self->owner->lpVtbl->Release(self->owner);
        free(self);
    }
    return remaining;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_GetIids(FFWinrtHstringIterator* self, std::uint32_t* count, GUID** iids) {
    (void) self;
    *count = 0;
    *iids = nullptr;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_GetRuntimeClassName(FFWinrtHstringIterator* self, void** name) {
    (void) self;
    *name = nullptr;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_GetTrustLevel(FFWinrtHstringIterator* self, std::int32_t* level) {
    (void) self;
    *level = 0;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_get_Current(FFWinrtHstringIterator* self, HSTRING* value) {
    if (!value) {
        return E_POINTER;
    }
    *value = nullptr;

    if (self->index >= self->owner->strings.length) {
        return E_BOUNDS;
    }

    // The caller owns what it gets, so hand out a duplicate rather than the stored string.
    return WindowsDuplicateString(*(HSTRING*) ffListGet(&self->owner->strings, sizeof(HSTRING), self->index), value);
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_get_HasCurrent(FFWinrtHstringIterator* self, bool* value) {
    if (!value) {
        return E_POINTER;
    }
    *value = self->index < self->owner->strings.length;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_MoveNext(FFWinrtHstringIterator* self, bool* value) {
    if (!value) {
        return E_POINTER;
    }

    if (self->index < self->owner->strings.length) {
        ++self->index;
    }
    *value = self->index < self->owner->strings.length;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE ffWinrtHstringIterator_GetMany(FFWinrtHstringIterator* self, std::uint32_t capacity, HSTRING* values, std::uint32_t* actual) {
    if (!actual) {
        return E_POINTER;
    }
    *actual = 0;

    if (capacity && !values) {
        return E_POINTER;
    }

    std::uint32_t available = self->owner->strings.length - self->index;
    std::uint32_t taken = capacity < available ? capacity : available;

    for (std::uint32_t i = 0; i < taken; ++i) {
        WindowsDuplicateString(*(HSTRING*) ffListGet(&self->owner->strings, sizeof(HSTRING), self->index + i), &values[i]);
    }

    self->index += taken;
    *actual = taken;

    return taken == capacity ? S_OK : S_FALSE;
}

const FFWinrtHstringIteratorVtbl ffWinrtHstringIteratorVtbl = {
    ffWinrtHstringIterator_QueryInterface,
    ffWinrtHstringIterator_AddRef,
    ffWinrtHstringIterator_Release,
    ffWinrtHstringIterator_GetIids,
    ffWinrtHstringIterator_GetRuntimeClassName,
    ffWinrtHstringIterator_GetTrustLevel,
    ffWinrtHstringIterator_get_Current,
    ffWinrtHstringIterator_get_HasCurrent,
    ffWinrtHstringIterator_MoveNext,
    ffWinrtHstringIterator_GetMany,
};

} // namespace

void ffDeleteHstring(HSTRING* value) {
    if (*value) {
        WindowsDeleteString(*value);
        *value = nullptr;
    }
}

void ffStrbufSetHstring(FFstrbuf* destination, HSTRING value) {
    uint32_t length = 0;
    const wchar_t* raw = WindowsGetStringRawBuffer(value, &length);
    ffStrbufSetNWS(destination, length, raw);
}

HRESULT ffWinrtCreateHstringIterable(const wchar_t* const* items, uint32_t count, abi_t<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>** result) {
    *result = nullptr;

    if (count > 0 && !items) {
        return E_INVALIDARG;
    }

    auto* iterable = (FFWinrtHstringIterable*) malloc(sizeof(FFWinrtHstringIterable));
    if (!iterable) {
        return E_OUTOFMEMORY;
    }

    iterable->lpVtbl = &ffWinrtHstringIterableVtbl;
    iterable->refCount = 1;
    ffListInitA(&iterable->strings, sizeof(HSTRING), count > 0 ? count : FF_LIST_DEFAULT_ALLOC);

    for (uint32_t i = 0; i < count; ++i) {
        HSTRING* slot = (HSTRING*) ffListAdd(&iterable->strings, sizeof(HSTRING));
        HRESULT hr = WindowsCreateString(items[i], (UINT32) ::wcslen(items[i]), slot);
        if (FAILED(hr)) {
            iterable->lpVtbl->Release(iterable);
            return hr;
        }
    }

    *result = reinterpret_cast<WinrtIterableAbi*>(iterable);
    return S_OK;
}

#endif // FF_HAVE_WINRT
