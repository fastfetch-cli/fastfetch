#pragma once

#ifndef FF_INCLUDED_UTIL_WCWIDTH_H
#define FF_INCLUDED_UTIL_WCWIDTH_H

#include <wchar.h>

#ifdef FF_HAVE_WCWIDTH
static inline int mk_wcwidth(wchar_t ucs) {
    return wcwidth(ucs);
}
static inline int mk_wcswidth(const wchar_t *pwcs, size_t n) {
    return wcswidth(pwcs, n);
}
#else
// https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
int mk_wcwidth(wchar_t ucs);
int mk_wcswidth(const wchar_t *pwcs, size_t n);
#endif

#endif
