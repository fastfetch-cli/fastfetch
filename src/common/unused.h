#pragma once

static inline void ffUnused(int dummy, ...) {
    (void) dummy;
}
#define FF_UNUSED(...) ffUnused(0, __VA_ARGS__);
