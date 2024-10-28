#pragma once

#ifdef FF_USE_STBPRINTF

#include <stdio.h>
#include <stdint.h>
#include "3rdparty/stb/stb_sprintf.h"

static inline char *STB_SPRINTF_DECORATE(printf_callback)(const char* __restrict buf, void* __restrict user, int len)
{
    fwrite(buf, 1, (uint32_t) len, (FILE *)user);
    return (char *) buf;
}

static inline int STB_SPRINTF_DECORATE(vfprintf)(FILE* file, char const* __restrict fmt, va_list va)
{
    char tmp[STB_SPRINTF_MIN];
    return STB_SPRINTF_DECORATE(vsprintfcb)(STB_SPRINTF_DECORATE(printf_callback), file, tmp, fmt, va);
}
#define vfprintf(...) STB_SPRINTF_DECORATE(vfprintf)(__VA_ARGS__)

static inline int STBSP__ATTRIBUTE_FORMAT(1,2) STB_SPRINTF_DECORATE(printf)(char const* __restrict fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int result = STB_SPRINTF_DECORATE(vfprintf)(stdout, fmt, va);
    va_end(va);
    return result;
}
#define printf(...) STB_SPRINTF_DECORATE(printf)(__VA_ARGS__)

static inline int STBSP__ATTRIBUTE_FORMAT(2,3) STB_SPRINTF_DECORATE(fprintf)(FILE* __restrict file, char const* __restrict fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int result = STB_SPRINTF_DECORATE(vfprintf)(file, fmt, va);
    va_end(va);
    return result;
}
#define fprintf(...) STB_SPRINTF_DECORATE(fprintf)(__VA_ARGS__)

#define vsprintf(...) STB_SPRINTF_DECORATE(vsprintf)(__VA_ARGS__)

#define vsnprintf(...) STB_SPRINTF_DECORATE(vsnprintf)(__VA_ARGS__)

#define sprintf(...) STB_SPRINTF_DECORATE(sprintf)(__VA_ARGS__)

#define snprintf(...) STB_SPRINTF_DECORATE(snprintf)(__VA_ARGS__)

#endif
