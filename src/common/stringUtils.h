#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "common/wcwidth.h"

static inline bool ffStrSet(const char* str) {
    if (str == NULL) {
        return false;
    }

    while (isspace(*str)) {
        str++;
    }

    return *str != '\0';
}

static inline bool ffStrStartsWithIgnCase(const char* str, const char* compareTo) {
    return strncasecmp(str, compareTo, strlen(compareTo)) == 0;
}

static inline bool ffStrEqualsIgnCase(const char* str, const char* compareTo) {
    return strcasecmp(str, compareTo) == 0;
}

static inline bool ffStrStartsWith(const char* str, const char* compareTo) {
    return strncmp(str, compareTo, strlen(compareTo)) == 0;
}

static inline bool ffStrEndsWith(const char* str, const char* compareTo) {
    size_t strLength = strlen(str);
    size_t compareToLength = strlen(compareTo);
    if (strLength < compareToLength) {
        return false;
    }
    return memcmp(str + strLength - compareToLength, compareTo, compareToLength) == 0;
}

static inline bool ffStrEndsWithIgnCase(const char* str, const char* compareTo) {
    size_t strLength = strlen(str);
    size_t compareToLength = strlen(compareTo);
    if (strLength < compareToLength) {
        return false;
    }
    return strncasecmp(str + strLength - compareToLength, compareTo, compareToLength) == 0;
}

static inline bool ffStrEquals(const char* str, const char* compareTo) {
    return strcmp(str, compareTo) == 0;
}

static inline bool ffStrContains(const char* str, const char* compareTo) {
    return strstr(str, compareTo) != NULL;
}

static inline bool ffStrContainsIgnCase(const char* str, const char* compareTo) {
    return strcasestr(str, compareTo) != NULL;
}

static inline bool ffStrContainsC(const char* str, char compareTo) {
    return strchr(str, compareTo) != NULL;
}

static inline bool ffCharIsEnglishAlphabet(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static inline bool ffCharIsDigit(char c) {
    return '0' <= c && c <= '9';
}

// Parse one UTF-8 character, returning consumed byte count and display width.
// Invalid / incomplete sequence falls back to one-byte width=1.
// If the Unicode codepoint is non-printable, width becomes 0.
static inline uint8_t ffUtf8CharLenWidth(const char* str, uint32_t length, uint8_t* width) {
    if (__builtin_expect(length == 0 || *str == '\0', false)) {
        if (width) {
            *width = 0;
        }
        return 0;
    }

    unsigned char first = (unsigned char) *str;
    if (__builtin_expect(first < 0x80, true)) {
        if (width) {
            *width = 1;
        }
        return 1;
    }

    uint8_t bytes;
    if ((first & 0xE0) == 0xC0) {
        bytes = 2;
    } else if ((first & 0xF0) == 0xE0) {
        bytes = 3;
    } else if ((first & 0xF8) == 0xF0) {
        bytes = 4;
    } else {
        if (width) {
            *width = 1;
        }
        return 1;
    }

    if (length < bytes) {
        if (width) {
            *width = 1;
        }
        return 1;
    }

    for (uint8_t i = 1; i < bytes; ++i) {
        unsigned char continuation = (unsigned char) str[i];
        if (continuation == '\0' || (continuation & 0xC0) != 0x80) {
            if (width) {
                *width = 1;
            }
            return 1;
        }
    }

    uint32_t ucs = (uint32_t) (first & ((1U << (8 - bytes)) - 1));
    for (uint8_t i = 1; i < bytes; ++i) {
        ucs <<= 6;
        ucs |= (uint32_t) ((unsigned char) str[i] & 0x3F);
    }

    int wcWidth = mk_wcwidth(ucs);
    if (width) {
        *width = (uint8_t) (wcWidth < 0 ? 0 : wcWidth);
    }
    return bytes;
}

static inline uint32_t ffUtf8StrWidth(const char* str, uint32_t length) {
    uint32_t result = 0;
    const char* ptr = str;

    while (length > 0 && *ptr != '\0') {
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth(ptr, length, &width);
        if (__builtin_expect(bytes == 0, false)) {
            break;
        }

        result += width;
        ptr += bytes;
        length -= bytes;
    }

    return result > 0 ? result : (uint32_t) (ptr - str);
}

static inline bool ffCharIsHexDigit(char c) {
    return ffCharIsDigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

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
static inline char* ffStrCopy(char* __restrict__ dst, const char* __restrict__ src, size_t dstBufSiz) {
    if (__builtin_expect(dst == NULL, false) || dstBufSiz == 0) {
        return dst;
    }

    size_t len = strnlen(src, dstBufSiz - 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst + len;
}
