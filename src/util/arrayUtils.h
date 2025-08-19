#pragma once

#include <assert.h>

#ifdef __has_builtin
    #if __has_builtin(__is_array)
        #define ARRAY_SIZE(x) ({ static_assert(__is_array(__typeof__(x)), "Must be an array"); (uint32_t) (sizeof(x) / sizeof(*(x))); })
    #elif __has_builtin(__builtin_types_compatible_p)
        #define ARRAY_SIZE(x) ({ static_assert(!__builtin_types_compatible_p(__typeof__(x), __typeof__(&*(x))), "Must not be a pointer"); (uint32_t) (sizeof(x) / sizeof(*(x))); })
    #endif
#endif
#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(x) ((uint32_t) (sizeof(x) / sizeof(*(x))))
#endif
