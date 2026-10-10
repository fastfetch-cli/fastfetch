#pragma once

#include <stdint.h>

// `__BIG_ENDIAN__` alone is not enough: GCC does not define it, it documents only `__BYTE_ORDER__`
// and the `__ORDER_*_ENDIAN__` values. Testing the former silently selected the little-endian
// branch on s390x, where `FF_READ_BE` then byte-swapped values that were already big-endian.
// `defined(__BIG_ENDIAN__)` alone would not do either: a macro that is defined as 0 is not a
// big-endian platform.
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || (defined(__BIG_ENDIAN__) && __BIG_ENDIAN__)
    #define FF_READ_LE(x) _Generic((x), \
        uint16_t: __builtin_bswap16(x), \
        uint32_t: __builtin_bswap32(x), \
        uint64_t: __builtin_bswap64(x))
    #define FF_READ_BE(x) (x)
#else
    #define FF_READ_LE(x) (x)
    #define FF_READ_BE(x) _Generic((x), \
        uint16_t: __builtin_bswap16(x), \
        uint32_t: __builtin_bswap32(x), \
        uint64_t: __builtin_bswap64(x))
#endif
