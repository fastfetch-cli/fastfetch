#include <errno.h>

#ifndef NDEBUG

    #include "common/debug.h"
    #include "common/thread.h"

[[gnu::nonnull(1), gnu::pure, gnu::always_inline, nodiscard]]
static inline const char* ffFindFileName(const char* file) {
    const char* lastSlash = __builtin_strrchr(file, '/');
    #ifdef _WIN32
    if (lastSlash == nullptr) {
        lastSlash = __builtin_strrchr(file, '\\');
    }
    #endif
    if (lastSlash != nullptr) {
        return lastSlash + 1;
    }
    return file;
}

#if __GNUC__ && !__clang__
[[gnu::optimize("O2")]]
#endif
void ffDebugPrint(const char* file, int line, const char* format, ...) {
    if (!instance.config.display.debugMode) {
        return;
    }

    static FFThreadMutex debugMutex = FF_THREAD_MUTEX_INITIALIZER;
    ffThreadMutexLock(&debugMutex);
    int errno_ = errno;
    static char errmsg_[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(errmsg_, sizeof(errmsg_), format, args);
    va_end(args);
    fprintf(stderr, "[%s%4d, %s] %s\n", ffFindFileName(file), line, ffTimeToTimeStr(ffTimeGetNow()), errmsg_);
    errno = errno_;
    ffThreadMutexUnlock(&debugMutex);
}

    #if _WIN32
        #include "common/windows/nt.h"

        #include <windows.h>
        #include <cfgmgr32.h>

const char* ffDebugWin32Error(DWORD errorCode) {
    static char buffer[512];

    wchar_t bufferW[256];
    ULONG len = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        (DWORD) errorCode,
        0,
        bufferW,
        ARRAY_SIZE(bufferW),
        nullptr);

    if (len == 0) {
        snprintf(buffer, sizeof(buffer), "Unknown error code (%lu)", errorCode);
    } else {
        // Remove trailing newline
        while (len > 0 && (bufferW[len - 1] == '\r' || bufferW[len - 1] == '\n')) {
            --len;
        }

        if (NT_SUCCESS(RtlUnicodeToUTF8N(buffer, sizeof(buffer), &len, bufferW, len * sizeof(wchar_t)))) {
            snprintf(buffer + len, sizeof(buffer) - len, " (%lu)", errorCode);
        } else {
            snprintf(buffer, sizeof(buffer), "Unknown error (%lu)", errorCode);
        }
    }

    return buffer;
}

const char* ffDebugConfigRet(CONFIGRET ret) {
    return ffDebugWin32Error(CM_MapCrToWin32Err(ret, ERROR_INTERNAL_ERROR));
}

const char* ffDebugNtStatus(NTSTATUS status) {
    return ffDebugWin32Error(RtlNtStatusToDosError(status));
}

static inline DWORD HRESULTToWin32Error(HRESULT hr) {
    if (SUCCEEDED(hr)) {
        return ERROR_SUCCESS;
    }

    if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
        return HRESULT_CODE(hr);
    }

    return ERROR_INTERNAL_ERROR;
}

const char* ffDebugHResult(HRESULT hr) {
    return ffDebugWin32Error(HRESULTToWin32Error(hr));
}
    #endif

#endif
