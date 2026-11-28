#pragma once

#include "common/FFstrbuf.h"
#include <wchar.h>

// ---------------------------------------------------------------------------------------------
// A wide-character string buffer for building UTF-16 paths. It follows the FFstrbuf
// Init / Create / Destroy / Append* conventions, but is always heap allocated: `capacity`
// counts wchar_t slots including the null terminator. A destroyed buffer (or the result of a
// failed conversion) is zeroed with `chars == nullptr`, which is safe to destroy again.
// ---------------------------------------------------------------------------------------------

#define FASTFETCH_WSTRBUF_DEFAULT_ALLOC 64

typedef struct FFWStrbuf {
    wchar_t* chars;
    uint32_t length;
    uint32_t capacity;
} FFWStrbuf;

[[gnu::nonnull(1)]] static inline void ffWStrbufEnsureFree(FFWStrbuf* strbuf, uint32_t free) {
    uint64_t required = (uint64_t) strbuf->length + free + 1; // +1 for the null terminator
    if (required <= strbuf->capacity) {
        return;
    }

    uint32_t capacity = strbuf->capacity == 0 ? FASTFETCH_WSTRBUF_DEFAULT_ALLOC : strbuf->capacity;
    while (capacity < required) {
        if (capacity > UINT32_MAX / 2) {
            capacity = (uint32_t) required;
            break;
        }
        capacity *= 2;
    }

    strbuf->chars = (wchar_t*) realloc(strbuf->chars, (size_t) capacity * sizeof(wchar_t));
    strbuf->capacity = capacity;
}

[[gnu::nonnull(1)]] static inline void ffWStrbufInit(FFWStrbuf* strbuf) {
    strbuf->chars = nullptr;
    strbuf->length = 0;
    strbuf->capacity = 0;
    ffWStrbufEnsureFree(strbuf, 0);
    strbuf->chars[0] = L'\0';
}

// Despite the name, resets the buffer to the initial, unallocated state
[[gnu::nonnull(1)]] static inline void ffWStrbufDestroy(FFWStrbuf* strbuf) {
    free(strbuf->chars);
    strbuf->chars = nullptr;
    strbuf->length = 0;
    strbuf->capacity = 0;
}

[[gnu::nonnull(1)]] static inline void ffWStrbufAppendNS(FFWStrbuf* strbuf, uint32_t length, const wchar_t* value) {
    ffWStrbufEnsureFree(strbuf, length);
    if (length > 0) {
        memcpy(strbuf->chars + strbuf->length, value, (size_t) length * sizeof(wchar_t));
    }
    strbuf->length += length;
    strbuf->chars[strbuf->length] = L'\0';
}

[[gnu::nonnull(1)]] static inline void ffWStrbufAppendC(FFWStrbuf* strbuf, wchar_t value) {
    ffWStrbufEnsureFree(strbuf, 1);
    strbuf->chars[strbuf->length++] = value;
    strbuf->chars[strbuf->length] = L'\0';
}

[[nodiscard]] static inline FFWStrbuf ffWStrbufCreate(void) {
    FFWStrbuf strbuf;
    ffWStrbufInit(&strbuf);
    return strbuf;
}

// Converts a null-terminated UTF-8 string into a new UTF-16 buffer. `chars` is nullptr if the
// conversion failed; the returned value is always safe to pass to ffWStrbufDestroy.
[[gnu::nonnull(1), nodiscard]] FFWStrbuf ffWStrbufCreateUtf8(const char* in);

#define FF_WSTRBUF_AUTO_DESTROY [[gnu::cleanup(ffWStrbufDestroy)]] FFWStrbuf

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source);
void ffStrbufAppendNWS(FFstrbuf* result, uint32_t length, const wchar_t* source);

static inline void ffStrbufSetWS(FFstrbuf* result, const wchar_t* source) {
    if (!source) {
        return ffStrbufClear(result);
    }
    return ffStrbufSetNWS(result, (uint32_t) wcslen(source), source);
}

static inline void ffStrbufAppendWS(FFstrbuf* result, const wchar_t* source) {
    if (!source) {
        return;
    }
    return ffStrbufAppendNWS(result, (uint32_t) wcslen(source), source);
}

static inline void ffStrbufInitNWS(FFstrbuf* result, uint32_t length, const wchar_t* source) {
    ffStrbufInit(result);
    return ffStrbufSetNWS(result, length, source);
}

static inline void ffStrbufInitWS(FFstrbuf* result, const wchar_t* source) {
    if (!source) {
        return ffStrbufInit(result);
    }
    return ffStrbufInitNWS(result, (uint32_t) wcslen(source), source);
}

static inline FFstrbuf ffStrbufCreateNWS(uint32_t length, const wchar_t* source) {
    FFstrbuf result;
    ffStrbufInitNWS(&result, length, source);
    return result;
}

static inline FFstrbuf ffStrbufCreateWS(const wchar_t* source) {
    if (!source) {
        return ffStrbufCreate();
    }
    return ffStrbufCreateNWS((uint32_t) wcslen(source), source);
}
