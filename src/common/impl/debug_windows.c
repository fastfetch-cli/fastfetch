#include "common/debug.h"
#include "common/windows/nt.h"

#include <windows.h>

const char* ffDebugWin32Error(DWORD errorCode) {
    static char buffer[512];

    wchar_t bufferW[256];
    ULONG len = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD) errorCode,
        0,
        bufferW,
        sizeof(buffer),
        NULL);

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
