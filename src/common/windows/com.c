#include "com.h"

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
                return "RoInitialize() failed: invalid argument";
            case E_OUTOFMEMORY:
                return "RoInitialize() failed: out of memory";
            case E_UNEXPECTED:
                return "RoInitialize() failed: unexpected error";
            case RPC_E_CHANGED_MODE:
                // COM was already initialized with a different concurrency model
                return NULL;
            default:
                return "RoInitialize() failed: unknown error";
        }
    }

    atexit(RoUninitializeWrap);
    return NULL;
}
#else
    #include <combaseapi.h>

static void CoUninitializeWrap(void) {
    CoUninitialize();
}

static const char* doInitCom() {
    HRESULT res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(res)) {
        switch (res) {
            case E_INVALIDARG:
                return "CoInitializeEx() failed: invalid argument";
            case E_OUTOFMEMORY:
                return "CoInitializeEx() failed: out of memory";
            case RPC_E_CHANGED_MODE:
                // COM was already initialized with a different concurrency model
                return NULL;
            default:
                return "CoInitializeEx() failed: unknown error";
        }
    }

    atexit(CoUninitializeWrap);
    return NULL;
}
#endif

const char* ffInitCom(void) {
    static const char* error = "";
    if (error && error[0] == '\0') {
        error = doInitCom();
    }
    return error;
}
