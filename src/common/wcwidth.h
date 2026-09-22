#pragma once

#include <stdint.h>

#if FF_ENABLE_WCWIDTH
// A pure table lookup: the width depends only on the code point
[[gnu::pure, nodiscard]] int mk_wcwidth(uint32_t wc);
#else
[[gnu::pure, nodiscard]] static inline int mk_wcwidth(uint32_t wc) {
    (void) wc;
    return 1;
}
#endif
