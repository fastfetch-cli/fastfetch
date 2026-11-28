#include "unicode.h"

#include "common/windows/nt.h"

FFWStrbuf ffWStrbufCreateUtf8(const char* in) {
    FFWStrbuf strbuf = { .chars = nullptr, .length = 0, .capacity = 0 };

    ULONG inputLength = (ULONG) strlen(in) + 1;
    ULONG size = 0;
    (void) RtlUTF8ToUnicodeN(nullptr, 0, &size, in, inputLength);
    if (size == 0) {
        return strbuf;
    }

    wchar_t* result = (wchar_t*) malloc(size);
    ULONG written = 0;
    if (!result || !NT_SUCCESS(RtlUTF8ToUnicodeN(result, size, &written, in, inputLength))) {
        free(result);
        return strbuf;
    }

    strbuf.chars = result;
    strbuf.capacity = (uint32_t) (size / sizeof(wchar_t));
    strbuf.length = (uint32_t) wcslen(result);
    return strbuf;
}

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source) {
    if (!length) {
        ffStrbufClear(result);
        return;
    }

    ULONG size_needed = 0;
    NTSTATUS status = RtlUnicodeToUTF8N(nullptr, 0, &size_needed, source, length * sizeof(wchar_t));

    if (size_needed == 0) {
        ffStrbufSetF(result, "RtlUnicodeToUTF8N failed: %X", (unsigned) status);
        return;
    }

    ffStrbufEnsureFixedLengthFree(result, size_needed);
    RtlUnicodeToUTF8N(result->chars, size_needed, &size_needed, source, length * sizeof(wchar_t));

    result->length = size_needed;
    result->chars[size_needed] = '\0';
}

void ffStrbufAppendNWS(FFstrbuf* result, uint32_t length, const wchar_t* source) {
    if (!length) {
        return;
    }

    ULONG size_needed = 0;
    NTSTATUS status = RtlUnicodeToUTF8N(nullptr, 0, &size_needed, source, length * sizeof(wchar_t));

    if (size_needed == 0) {
        ffStrbufAppendF(result, "RtlUnicodeToUTF8N failed: %X", (unsigned) status);
        return;
    }

    ffStrbufEnsureFree(result, size_needed);
    RtlUnicodeToUTF8N(result->chars + result->length, size_needed, &size_needed, source, length * sizeof(wchar_t));

    result->length += size_needed;
    result->chars[result->length] = '\0';
}
