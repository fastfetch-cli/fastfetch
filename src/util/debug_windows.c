#include "debug.h"

#include <windows.h>

const char* ffDebugWin32Error(DWORD errorCode)
{
    static char buffer[256];

    DWORD len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD) errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        sizeof(buffer),
        NULL);

    if (len == 0) {
        snprintf(buffer, sizeof(buffer), "Unknown error code (%lu)", errorCode);
    } else {
        // Remove trailing newline
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        if (buffer[len - 2] == '\r') buffer[len - 2] = '\0';
        snprintf(buffer + len - 2, sizeof(buffer) - len + 2, " (%lu)", errorCode);
    }

    return buffer;
}
