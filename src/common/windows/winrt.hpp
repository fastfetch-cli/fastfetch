#pragma once

#ifdef __cplusplus

extern "C" {
    #include "common/FFlist.h"
    #include "common/FFstrbuf.h"
    #include "common/time.h"
    #include "common/windows/com.h"
    #include "common/windows/unicode.h"
}

    #if FF_HAVE_WINRT

        #include <roapi.h>
        #include <winstring.h>
        #include <asyncinfo.h>

        #include <winrt/base.h>
        #include <winrt/Windows.Foundation.h>
        #include <winrt/Windows.Foundation.Collections.h>

using winrt::impl::abi_t;

// Frees an HSTRING and clears the pointer, so it can be used with [[gnu::cleanup]].
static inline void ffDeleteHstring(HSTRING* value) {
    if (*value) {
        WindowsDeleteString(*value);
        *value = nullptr;
    }
}

// Copies an HSTRING into an FFstrbuf, transcoding from UTF-16 to UTF-8.
static inline void ffStrbufSetHstring(FFstrbuf* destination, HSTRING value) {
    uint32_t length = 0;
    const wchar_t* raw = WindowsGetStringRawBuffer(value, &length);
    ffStrbufSetNWS(destination, length, raw);
}

// Wraps `items` in an IVector<hstring> and hands out its IIterable<hstring> view, which is the
// parameter shape the FindAllAsync overloads taking additional properties expect. The caller owns
// the result.
HRESULT ffWinrtCreateHstringIterable(const wchar_t* const* items, uint32_t count, abi_t<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>** result);

// RoGetActivationFactory() for a WinRT runtime class.
//
// `Projection` must be the *projection* type -- e.g.
// winrt::Windows::Devices.Bluetooth::IBluetoothLEDeviceStatics -- and never abi_t<...>:
// winrt::impl::guid_v is only specialized for projection types, and abi_t<T> expands to
// impl::abi<T>::type, which is a non-deduced context.
template <typename Projection>
static inline HRESULT ffGetActivationFactory(const wchar_t* className, abi_t<Projection>** factory) {
    HSTRING_HEADER header;
    HSTRING runtimeClass;
    HRESULT hr = WindowsCreateStringReference(className, (UINT32) ::wcslen(className), &header, &runtimeClass);
    if (FAILED(hr)) {
        return hr;
    }

    return RoGetActivationFactory(runtimeClass, winrt::guid_of<Projection>(), reinterpret_cast<void**>(factory));
}

template <typename TargetProjection, typename SourceAbi>
static inline HRESULT ffQueryInterface(SourceAbi* source, abi_t<TargetProjection>** target) {
    return source->QueryInterface(winrt::guid_of<TargetProjection>(), reinterpret_cast<void**>(target));
}

// Spins until `operation` leaves AsyncStatus::Started, then hands its results to `result`.
template <typename TOperationAbi, typename TResultAbi>
static HRESULT ffWaitForAsyncOperation(TOperationAbi* operation, TResultAbi** result) {
    FF_AUTO_RELEASE_COM_OBJECT IAsyncInfo* asyncInfo = nullptr;
    HRESULT hr = ffQueryInterface<IAsyncInfo>(operation, &asyncInfo);
    if (FAILED(hr)) {
        return hr;
    }

    AsyncStatus status = AsyncStatus::Started;

    for (;;) {
        hr = asyncInfo->get_Status(&status);
        if (FAILED(hr)) {
            return hr;
        }
        if (status == AsyncStatus::Started) {
            ffTimeSleep(0);
        } else {
            break;
        }
    }

    if (status != AsyncStatus::Completed) {
        HRESULT errorCode = E_FAIL;
        asyncInfo->get_ErrorCode(&errorCode);
        return FAILED(errorCode) ? errorCode : E_FAIL;
    }

    return operation->GetResults((void**) result);
}

template <typename TResultProjection, typename TOperation>
static HRESULT ffRunAndWait(TOperation&& operation, abi_t<TResultProjection>** result) {
    FF_AUTO_RELEASE_COM_OBJECT abi_t<winrt::Windows::Foundation::IAsyncOperation<TResultProjection>>* opResult = nullptr;
    HRESULT hr = operation(reinterpret_cast<void**>(&opResult));
    if (FAILED(hr) || !opResult) {
        return hr;
    }

    return ffWaitForAsyncOperation(opResult, result);
}

template <typename TResultProjection, typename TOperation>
static HRESULT ffRunAndWait2(TOperation&& operation, abi_t<TResultProjection>** result) {
    *result = nullptr;

    FF_AUTO_RELEASE_COM_OBJECT abi_t<winrt::Windows::Foundation::IAsyncOperationWithProgress<TResultProjection, int32_t>>* opResult = nullptr;
    HRESULT hr = operation(reinterpret_cast<void**>(&opResult));
    if (FAILED(hr) || !opResult) {
        return hr;
    }

    return ffWaitForAsyncOperation(opResult, result);
}

    #endif // FF_HAVE_WINRT

#else

    #error Must be included in C++ source file

#endif
