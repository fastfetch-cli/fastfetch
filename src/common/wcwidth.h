#pragma once

#include <stdint.h>

#if FF_ENABLE_WCWIDTH
int mk_wcwidth(uint32_t wc);
#else
static inline int mk_wcwidth(uint32_t wc) {
    (void) wc;
    return 1;
}
#endif
