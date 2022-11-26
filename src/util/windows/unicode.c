#include "unicode.h"

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source)
{
    if(!length)
    {
        ffStrbufClear(result);
        return;
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, source, (int)length, NULL, 0, NULL, NULL);
    ffStrbufEnsureFree(result, (uint32_t)size_needed);
    WideCharToMultiByte(CP_UTF8, 0, source, (int)length, result->chars, size_needed, NULL, NULL);
    result->length = (uint32_t)size_needed;
    result->chars[size_needed] = '\0';
}

FFstrbuf ffStrbufCreateNWS(uint32_t length, const wchar_t* source)
{
    FFstrbuf result;

    if(length == 0)
        ffStrbufInit(&result);
    else
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, source, (int)length, NULL, 0, NULL, NULL);
        ffStrbufInitA(&result, (uint32_t)size_needed);
        WideCharToMultiByte(CP_UTF8, 0, source, (int)length, result.chars, size_needed, NULL, NULL);
        result.length = (uint32_t)size_needed;
        result.chars[size_needed] = '\0';
    }

    return result;
}
