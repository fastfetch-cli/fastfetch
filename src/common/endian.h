#pragma once

#include <stdint.h>

#if __BIG_ENDIAN__
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
