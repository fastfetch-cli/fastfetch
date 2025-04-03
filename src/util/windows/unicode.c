#include "unicode.h"

#include <windows.h>

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source)
{
    if(!length)
    {
        ffStrbufClear(result);
        return;
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, source, (int)length, NULL, 0, NULL, NULL);
    if (size_needed < 0)
    {
        ffStrbufSetF(result, "WCTMB failed: %u", (unsigned) GetLastError());
        return;
    }
    ffStrbufEnsureFixedLengthFree(result, (uint32_t)size_needed);
    WideCharToMultiByte(CP_UTF8, 0, source, (int)length, result->chars, size_needed, NULL, NULL);
    result->length = (uint32_t)size_needed;
    result->chars[size_needed] = '\0';
}
