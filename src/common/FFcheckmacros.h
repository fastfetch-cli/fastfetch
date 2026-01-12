#pragma once

#ifdef _MSC_VER
    #include <sal.h>
#endif

#if defined(__has_attribute) && __has_attribute(__warn_unused_result__)
    #define FF_C_NODISCARD __attribute__((__warn_unused_result__))
#elif defined(_MSC_VER)
    #define FF_C_NODISCARD _Check_return_
#else
    #define FF_C_NODISCARD
#endif

#if defined(__has_attribute) && __has_attribute(__format__)
    #define FF_C_PRINTF(formatStrIndex, argsStartIndex) __attribute__((__format__ (printf, formatStrIndex, argsStartIndex)))
#else
    #define FF_C_PRINTF(formatStrIndex, argsStartIndex)
#endif

#if defined(__has_attribute) && __has_attribute(__format__)
    #define FF_C_SCANF(formatStrIndex, argsStartIndex) __attribute__((__format__ (scanf, formatStrIndex, argsStartIndex)))
#else
    #define FF_C_SCANF(formatStrIndex, argsStartIndex)
#endif

#if defined(__has_attribute) && __has_attribute(__nonnull__)
    #define FF_C_NONNULL(argIndex, ...) __attribute__((__nonnull__(argIndex, ##__VA_ARGS__)))
#else
    #define FF_C_NONNULL(argIndex, ...)
#endif

#if defined(__has_attribute) && __has_attribute(__returns_nonnull__)
    #define FF_C_RETURNS_NONNULL __attribute__((__returns_nonnull__))
#else
    #define FF_C_RETURNS_NONNULL
#endif
