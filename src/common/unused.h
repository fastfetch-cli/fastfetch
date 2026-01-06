#pragma once

static inline void ffUnused(int dummy, ...) { (void) dummy; }
#define FF_UNUSED(...) ffUnused(0, __VA_ARGS__);
#if defined(__GNUC__) || defined(__clang__)
    #define FF_MAYBE_UNUSED __attribute__ ((__unused__))
#else
    #define FF_MAYBE_UNUSED
#endif
