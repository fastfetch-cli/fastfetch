#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
// #include <shlwapi.h>
__stdcall char* StrStrIA(const char* lpFirst, const char* lpSrch);
    #define strcasestr StrStrIA
#endif

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

// Everything below is a leaf predicate over the bytes it is given: it reads memory through its
// arguments and nothing else, so `gnu::pure` is accurate. `gnu::const` is deliberately not used --
// these are already `always_inline`, so it would buy no optimization, and it would become a lie the
// day one of them is rewritten around a lookup table.
//
// The `gnu::nonnull` indexes name the arguments that are dereferenced unconditionally. `ffStrSet`
// and `ffStrCopy` are not listed: both handle a null pointer explicitly.

[[nodiscard, gnu::pure]]
static inline bool ffStrSet(const char* str) {
    if (str == nullptr) {
        return false;
    }

    while (isspace(*str)) {
        str++;
    }

    return *str != '\0';
}

[[gnu::always_inline, gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrStartsWithIgnCase(const char* str, const char* compareTo) {
    return strncasecmp(str, compareTo, strlen(compareTo)) == 0;
}

[[gnu::always_inline, gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrEqualsIgnCase(const char* str, const char* compareTo) {
    return strcasecmp(str, compareTo) == 0;
}

[[gnu::always_inline, gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrStartsWith(const char* str, const char* compareTo) {
    return strncmp(str, compareTo, strlen(compareTo)) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrEndsWith(const char* str, const char* compareTo) {
    size_t strLength = strlen(str);
    size_t compareToLength = strlen(compareTo);
    if (strLength < compareToLength) {
        return false;
    }
    return memcmp(str + strLength - compareToLength, compareTo, compareToLength) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrEndsWithIgnCase(const char* str, const char* compareTo) {
    size_t strLength = strlen(str);
    size_t compareToLength = strlen(compareTo);
    if (strLength < compareToLength) {
        return false;
    }
    return strncasecmp(str + strLength - compareToLength, compareTo, compareToLength) == 0;
}

[[gnu::always_inline, gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrEquals(const char* str, const char* compareTo) {
    return strcmp(str, compareTo) == 0;
}

[[gnu::always_inline, gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrContains(const char* str, const char* compareTo) {
    return strstr(str, compareTo) != nullptr;
}

[[gnu::always_inline, gnu::nonnull(1, 2), gnu::pure, nodiscard]]
static inline bool ffStrContainsIgnCase(const char* str, const char* compareTo) {
    return strcasestr(str, compareTo) != nullptr;
}

[[gnu::always_inline, gnu::nonnull(1), gnu::pure, nodiscard]]
static inline bool ffStrContainsC(const char* str, char compareTo) {
    return strchr(str, compareTo) != nullptr;
}

[[gnu::always_inline, gnu::pure, nodiscard]]
static inline bool ffCharIsEnglishAlphabet(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

[[gnu::always_inline, gnu::pure, nodiscard]]
static inline bool ffCharIsDigit(char c) {
    return '0' <= c && c <= '9';
}

// Parse one UTF-8 character, returning consumed byte count and display width.
// Invalid / incomplete sequence falls back to one-byte width=1.
// If the Unicode codepoint is non-printable, width becomes 0.
// Not `pure`: it stores the width through `width` when that is not null.
[[gnu::nonnull(1)]]
uint8_t ffUtf8CharLenWidth(const char* str, uint32_t length, uint8_t* width);

[[gnu::nonnull(1), gnu::pure, nodiscard]]
uint32_t ffUtf8StrWidth(const char* str, uint32_t length);

[[gnu::always_inline, gnu::pure, nodiscard]]
static inline bool ffCharIsHexDigit(char c) {
    return ffCharIsDigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

[[gnu::always_inline, gnu::pure, nodiscard]]
static inline int8_t ffHexCharToInt(char c) {
    if (ffCharIsDigit(c)) {
        return (int8_t) (c - '0');
    } else if ('a' <= c && c <= 'f') {
        return (int8_t) (c - 'a' + 10);
    } else if ('A' <= c && c <= 'F') {
        return (int8_t) (c - 'A' + 10);
    } else {
        return -1;
    }
}

// Copies at most (dstBufSiz - 1) bytes from src to dst; dst is always null-terminated
// Returns a pointer to the end of the copy, which callers are free to ignore, so not `nodiscard`.
// Not `pure`: it writes through `dst`. `dst` may be null, `src` may not.
[[gnu::nonnull(2)]]
static inline char* ffStrCopy(char* __restrict__ dst, const char* __restrict__ src, size_t dstBufSiz) {
    if (__builtin_expect(dst == nullptr, false) || dstBufSiz == 0) {
        return dst;
    }

    size_t len = strnlen(src, dstBufSiz - 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst + len;
}
