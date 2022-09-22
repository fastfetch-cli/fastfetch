#pragma once

#ifndef FASTFETCH_INCLUDED_FFCHECKMACROS
#define FASTFETCH_INCLUDED_FFCHECKMACROS

#if defined(__GNUC__) || defined(__clang__)
    #define FF_C_NODISCARD __attribute__((warn_unused_result))
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

#endif
