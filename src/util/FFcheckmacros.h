#pragma once

#ifdef _MSC_VER
    #include <sal.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define FF_C_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
    #define FF_C_NODISCARD _Check_return_
#else
    #define FF_C_NODISCARD
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define FF_C_PRINTF(formatStrIndex, argsStartIndex) __attribute__((__format__ (printf, formatStrIndex, argsStartIndex)))
#else
    #define FF_C_PRINTF(formatStrIndex, argsStartIndex)
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define FF_C_SCANF(formatStrIndex, argsStartIndex) __attribute__((__format__ (scanf, formatStrIndex, argsStartIndex)))
#else
    #define FF_C_SCANF(formatStrIndex, argsStartIndex)
#endif
