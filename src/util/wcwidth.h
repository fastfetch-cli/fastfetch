#pragma once

#include <wchar.h>

#ifndef FF_HAVE_WCWIDTH
#include "3rdparty/wcwidch/wcwidth.h"

static inline int wcswidth(const wchar_t *pwcs, size_t n)
{
    int res = 0;
    while (n-- > 0)
        res += wcwidth(*pwcs++);
    return res;
}
#endif
