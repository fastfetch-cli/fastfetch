#pragma once

#include <stdint.h>

#ifdef FF_HAVE_WCWIDTH
#include <wchar.h>

// Should be char32_t but it's not defined on macOS
static_assert(sizeof(wchar_t) == sizeof(uint32_t), "wcwidth implementation requires wchar_t to be 32 bits");

static inline int mk_wcwidth(uint32_t ucs) {
    return wcwidth((wchar_t) ucs);
}
#else
int mk_wcwidth(uint32_t wc);
#endif
