#include "unicode.h"

#include "common/windows/nt.h"

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source)
{
    if(!length)
    {
        ffStrbufClear(result);
        return;
    }

    ULONG size_needed = 0;
    NTSTATUS status = RtlUnicodeToUTF8N(NULL, 0, &size_needed, source, length * sizeof(wchar_t));

    if (size_needed == 0)
    {
        ffStrbufSetF(result, "RtlUnicodeToUTF8N failed: %X", (unsigned) status);
        return;
    }

    ffStrbufEnsureFixedLengthFree(result, size_needed);
    RtlUnicodeToUTF8N(result->chars, size_needed, &size_needed, source, length * sizeof(wchar_t));

    result->length = size_needed;
    result->chars[size_needed] = '\0';
}

void ffStrbufAppendNWS(FFstrbuf* result, uint32_t length, const wchar_t* source)
{
    if(!length)
        return;

    ULONG size_needed = 0;
    NTSTATUS status = RtlUnicodeToUTF8N(NULL, 0, &size_needed, source, length * sizeof(wchar_t));

    if (size_needed == 0)
    {
        ffStrbufAppendF(result, "RtlUnicodeToUTF8N failed: %X", (unsigned) status);
        return;
    }

    ffStrbufEnsureFree(result, size_needed);
    RtlUnicodeToUTF8N(result->chars + result->length, size_needed, &size_needed, source, length * sizeof(wchar_t));

    result->length += size_needed;
    result->chars[result->length] = '\0';
}
