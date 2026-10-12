#include "com.h"
#include "common/debug.h"

#include <stdlib.h>

#if FF_HAVE_WINRT
    #include <roapi.h>

static void RoUninitializeWrap(void) {
    RoUninitialize();
}

static const char* doInitCom() {
    HRESULT res = RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(res)) {
        switch (res) {
            case E_INVALIDARG:
                FF_DEBUG("RoInitialize() failed: invalid argument: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "RoInitialize() failed: invalid argument";
            case E_OUTOFMEMORY:
                FF_DEBUG("RoInitialize() failed: out of memory: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "RoInitialize() failed: out of memory";
            case E_UNEXPECTED:
                FF_DEBUG("RoInitialize() failed: unexpected error: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "RoInitialize() failed: unexpected error";
            case RPC_E_CHANGED_MODE:
                // COM was already initialized with a different concurrency model
                return nullptr;
            default:
                FF_DEBUG("RoInitialize() failed: unknown error: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "RoInitialize() failed: unknown error";
        }
    }

    atexit(RoUninitializeWrap);
    return nullptr;
}
#else
    #include <combaseapi.h>

static void CoUninitializeWrap(void) {
    CoUninitialize();
}

static const char* doInitCom() {
    HRESULT res = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(res)) {
        switch (res) {
            case E_INVALIDARG:
                FF_DEBUG("CoInitializeEx() failed: invalid argument: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "CoInitializeEx() failed: invalid argument";
            case E_OUTOFMEMORY:
                FF_DEBUG("CoInitializeEx() failed: out of memory: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "CoInitializeEx() failed: out of memory";
            case RPC_E_CHANGED_MODE:
                // COM was already initialized with a different concurrency model
                return nullptr;
            default:
                FF_DEBUG("CoInitializeEx() failed: unknown error: 0x%08lX (%s)", res, ffDebugHResult(res));
                return "CoInitializeEx() failed: unknown error";
        }
    }

    atexit(CoUninitializeWrap);
    return nullptr;
}
#endif

const char* ffInitCom(void) {
    static const char* error = "";
    if (error && error[0] == '\0') {
        error = doInitCom();
    }
    return error;
}
