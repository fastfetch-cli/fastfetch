#include "winrt.hpp"

#if FF_HAVE_WINRT

    #include <wchar.h>

#if defined(__GNUC__) || defined(__clang__)
extern "C" [[noreturn]] void __cxa_pure_virtual(void) {
    __builtin_trap();
}
#endif

// C++/WinRT supplies the ABI vtable shape, including IUnknown and IInspectable. These concrete
// classes only provide the implementation-specific state and methods; placement new keeps their
// storage under the C allocator used by the rest of fastfetch.
namespace {

using WinrtIterableAbi = abi_t<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>;
using WinrtIteratorAbi = abi_t<winrt::Windows::Foundation::Collections::IIterator<winrt::hstring>>;
using WinrtHstringAbiOut = winrt::impl::arg_out<winrt::hstring>;

struct FFWinrtHstringIterable;
struct FFWinrtHstringIterator;

struct FFWinrtHstringIterable final : WinrtIterableAbi {
    std::uint32_t refCount;
    FFlist strings; // HSTRING

    std::int32_t STDMETHODCALLTYPE QueryInterface(winrt::guid const& riid, void** result) noexcept override;
    std::uint32_t STDMETHODCALLTYPE AddRef() noexcept override;
    std::uint32_t STDMETHODCALLTYPE Release() noexcept override;
    std::int32_t STDMETHODCALLTYPE GetIids(std::uint32_t* count, winrt::guid** iids) noexcept override;
    std::int32_t STDMETHODCALLTYPE GetRuntimeClassName(void** name) noexcept override;
    std::int32_t STDMETHODCALLTYPE GetTrustLevel(winrt::Windows::Foundation::TrustLevel* level) noexcept override;
    std::int32_t STDMETHODCALLTYPE First(void** result) noexcept override;
};

struct FFWinrtHstringIterator final : WinrtIteratorAbi {
    std::uint32_t refCount;
    FFWinrtHstringIterable* owner;
    std::uint32_t index;

    std::int32_t STDMETHODCALLTYPE QueryInterface(winrt::guid const& riid, void** result) noexcept override;
    std::uint32_t STDMETHODCALLTYPE AddRef() noexcept override;
    std::uint32_t STDMETHODCALLTYPE Release() noexcept override;
    std::int32_t STDMETHODCALLTYPE GetIids(std::uint32_t* count, winrt::guid** iids) noexcept override;
    std::int32_t STDMETHODCALLTYPE GetRuntimeClassName(void** name) noexcept override;
    std::int32_t STDMETHODCALLTYPE GetTrustLevel(winrt::Windows::Foundation::TrustLevel* level) noexcept override;
    std::int32_t STDMETHODCALLTYPE get_Current(WinrtHstringAbiOut value) noexcept override;
    std::int32_t STDMETHODCALLTYPE get_HasCurrent(bool* value) noexcept override;
    std::int32_t STDMETHODCALLTYPE MoveNext(bool* value) noexcept override;
    std::int32_t STDMETHODCALLTYPE GetMany(std::uint32_t capacity, WinrtHstringAbiOut values, std::uint32_t* actual) noexcept override;
};

// ---------------------------------------------------------------------------------------------
// IIterable<HSTRING>
// ---------------------------------------------------------------------------------------------

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterable::QueryInterface(winrt::guid const& riid, void** result) noexcept {
    if (!result) {
        return E_POINTER;
    }
    *result = nullptr;

    if (riid == winrt::guid_of<winrt::Windows::Foundation::IUnknown>() ||
        riid == winrt::guid_of<winrt::Windows::Foundation::IInspectable>() ||
        riid == winrt::guid_of<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>()) {
        *result = this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

std::uint32_t STDMETHODCALLTYPE FFWinrtHstringIterable::AddRef() noexcept {
    return ++refCount;
}

std::uint32_t STDMETHODCALLTYPE FFWinrtHstringIterable::Release() noexcept {
    std::uint32_t remaining = --refCount;
    if (remaining == 0) {
        FF_LIST_FOR_EACH(HSTRING, string, strings) {
            WindowsDeleteString(*string);
        }
        ffListDestroy(&strings);
        this->~FFWinrtHstringIterable();
        free(this);
    }
    return remaining;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterable::GetIids(std::uint32_t* count, winrt::guid** iids) noexcept {
    *count = 0;
    *iids = nullptr;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterable::GetRuntimeClassName(void** name) noexcept {
    *name = nullptr;
    return S_OK;
}

// `level` is the ABI's 32-bit TrustLevel enum; BaseTrust is 0.
std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterable::GetTrustLevel(winrt::Windows::Foundation::TrustLevel* level) noexcept {
    *level = winrt::Windows::Foundation::TrustLevel::BaseTrust;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterable::First(void** result) noexcept {
    if (!result) {
        return E_POINTER;
    }
    *result = nullptr;

    auto* iterator = (FFWinrtHstringIterator*) malloc(sizeof(FFWinrtHstringIterator));
    if (!iterator) {
        return E_OUTOFMEMORY;
    }

    ::new (iterator) FFWinrtHstringIterator();
    iterator->refCount = 1;
    iterator->owner = this;
    iterator->index = 0;

    // The iterator outlives the call that created it, so it holds a reference of its own.
    AddRef();

    *result = iterator;
    return S_OK;
}

// ---------------------------------------------------------------------------------------------
// IIterator<HSTRING>
// ---------------------------------------------------------------------------------------------

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::QueryInterface(winrt::guid const& riid, void** result) noexcept {
    if (!result) {
        return E_POINTER;
    }
    *result = nullptr;

    if (riid == winrt::guid_of<winrt::Windows::Foundation::IUnknown>() ||
        riid == winrt::guid_of<winrt::Windows::Foundation::IInspectable>() ||
        riid == winrt::guid_of<winrt::Windows::Foundation::Collections::IIterator<winrt::hstring>>()) {
        *result = this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

std::uint32_t STDMETHODCALLTYPE FFWinrtHstringIterator::AddRef() noexcept {
    return ++refCount;
}

std::uint32_t STDMETHODCALLTYPE FFWinrtHstringIterator::Release() noexcept {
    std::uint32_t remaining = --refCount;
    if (remaining == 0) {
        owner->Release();
        this->~FFWinrtHstringIterator();
        free(this);
    }
    return remaining;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::GetIids(std::uint32_t* count, winrt::guid** iids) noexcept {
    *count = 0;
    *iids = nullptr;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::GetRuntimeClassName(void** name) noexcept {
    *name = nullptr;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::GetTrustLevel(winrt::Windows::Foundation::TrustLevel* level) noexcept {
    *level = winrt::Windows::Foundation::TrustLevel::BaseTrust;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::get_Current(WinrtHstringAbiOut value) noexcept {
    if (!value) {
        return E_POINTER;
    }
    *value = nullptr;

    if (index >= owner->strings.length) {
        return E_BOUNDS;
    }

    // The caller owns what it gets, so hand out a duplicate rather than the stored string.
    return WindowsDuplicateString(*(HSTRING*) ffListGet(&owner->strings, sizeof(HSTRING), index), reinterpret_cast<HSTRING*>(value));
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::get_HasCurrent(bool* value) noexcept {
    if (!value) {
        return E_POINTER;
    }
    *value = index < owner->strings.length;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::MoveNext(bool* value) noexcept {
    if (!value) {
        return E_POINTER;
    }

    if (index < owner->strings.length) {
        ++index;
    }
    *value = index < owner->strings.length;
    return S_OK;
}

std::int32_t STDMETHODCALLTYPE FFWinrtHstringIterator::GetMany(std::uint32_t capacity, WinrtHstringAbiOut values, std::uint32_t* actual) noexcept {
    if (!actual) {
        return E_POINTER;
    }
    *actual = 0;

    if (capacity && !values) {
        return E_POINTER;
    }

    std::uint32_t available = owner->strings.length - index;
    std::uint32_t taken = capacity < available ? capacity : available;
    HSTRING* hstrings = reinterpret_cast<HSTRING*>(values);

    for (std::uint32_t i = 0; i < taken; ++i) {
        WindowsDuplicateString(*(HSTRING*) ffListGet(&owner->strings, sizeof(HSTRING), index + i), &hstrings[i]);
    }

    index += taken;
    *actual = taken;

    return taken == capacity ? S_OK : S_FALSE;
}

} // namespace

HRESULT ffWinrtCreateHstringIterable(const wchar_t* const* items, uint32_t count, abi_t<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>** result) {
    *result = nullptr;

    if (count > 0 && !items) {
        return E_INVALIDARG;
    }

    auto* iterable = (FFWinrtHstringIterable*) malloc(sizeof(FFWinrtHstringIterable));
    if (!iterable) {
        return E_OUTOFMEMORY;
    }

    ::new (iterable) FFWinrtHstringIterable();
    iterable->refCount = 1;
    ffListInitA(&iterable->strings, sizeof(HSTRING), count > 0 ? count : FF_LIST_DEFAULT_ALLOC);

    for (uint32_t i = 0; i < count; ++i) {
        HSTRING* slot = (HSTRING*) ffListAdd(&iterable->strings, sizeof(HSTRING));
        HRESULT hr = WindowsCreateString(items[i], (UINT32) ::wcslen(items[i]), slot);
        if (FAILED(hr)) {
            iterable->Release();
            return hr;
        }
    }

    *result = static_cast<WinrtIterableAbi*>(iterable);
    return S_OK;
}

#endif // FF_HAVE_WINRT
