#include "unicode.h"

void ffWcharToUtf8(const wchar_t* input, FFstrbuf* result)
{
    int len = (int)wcslen(input);
    if(len <= 0)
    {
        ffStrbufClear(result);
        return;
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, input, len, NULL, 0, NULL, NULL);
    ffStrbufEnsureFree(result, (uint32_t)size_needed);
    WideCharToMultiByte(CP_UTF8, 0, input, len, result->chars, size_needed, NULL, NULL);
    result->length = (uint32_t)size_needed;
    result->chars[size_needed] = '\0';
}

FFstrbuf ffStrbufFromWchar(const wchar_t* input)
{
    FFstrbuf result;

    int len = input ? (int)wcslen(input) : 0;
    if(len <= 0)
        ffStrbufInit(&result);
    else
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, input, len, NULL, 0, NULL, NULL);
        ffStrbufInitA(&result, (uint32_t)size_needed);
        WideCharToMultiByte(CP_UTF8, 0, input, len, result.chars, size_needed, NULL, NULL);
        result.length = (uint32_t)size_needed;
        result.chars[size_needed] = '\0';
    }

    return result;
}
