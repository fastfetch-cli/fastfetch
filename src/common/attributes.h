#pragma once

#ifdef _MSC_VER
#    define __attribute__(x)
#endif

#define FF_A_FALLTHROUGH __attribute__((__fallthrough__))
#define FF_A_DEPRECATED __attribute__((__deprecated__))
#define FF_A_CLEANUP(func) __attribute__((__cleanup__(func)))
#define FF_A_NODISCARD __attribute__((__warn_unused_result__))
#define FF_A_PRINTF(formatStrIndex, argsStartIndex) __attribute__((__format__(printf, formatStrIndex, argsStartIndex)))
#define FF_A_SCANF(formatStrIndex, argsStartIndex) __attribute__((__format__(scanf, formatStrIndex, argsStartIndex)))
#define FF_A_NONNULL(argIndex, ...) __attribute__((__nonnull__(argIndex, ##__VA_ARGS__)))
#define FF_A_RETURNS_NONNULL __attribute__((__returns_nonnull__))
#define FF_A_UNUSED __attribute__((__unused__))
#define FF_A_PACKED __attribute__((__packed__))
#define FF_A_WEAK_IMPORT __attribute__((__weak_import__))
