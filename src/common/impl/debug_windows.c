#include "common/debug.h"

#include <windows.h>

const char* ffDebugWin32Error(DWORD errorCode)
{
    static char buffer[256];

    DWORD len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD) errorCode,
        0,
        buffer,
        sizeof(buffer),
        NULL);

    if (len == 0) {
        snprintf(buffer, sizeof(buffer), "Unknown error code (%lu)", errorCode);
    } else {
        // Remove trailing newline
        while (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n')) {
            buffer[--len] = '\0';
        }
        snprintf(buffer + len, sizeof(buffer) - len + 2, " (%lu)", errorCode);
    }

    return buffer;
}

const char* ffDebugNtStatus(NTSTATUS status)
{
    return ffDebugWin32Error(RtlNtStatusToDosError(status));
}
