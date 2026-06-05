#include "common/strutil.h"

uint8_t ffUtf8CharLenWidth(const char* str, uint32_t length, uint8_t* width) {
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

uint32_t ffUtf8StrWidth(const char* str, uint32_t length) {
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
