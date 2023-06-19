#pragma once

#ifndef FF_INCLUDED_util_stringUtils_h
#define FF_INCLUDED_util_stringUtils_h

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

bool ffStrSet(const char* str);
bool ffStrHasNChars(const char* str, char c, uint32_t n);

static inline bool ffStrStartsWithIgnCase(const char* str, const char* compareTo)
{
    return strncasecmp(str, compareTo, strlen(compareTo)) == 0;
}

static inline bool ffStrEqualsIgnCase(const char* str, const char* compareTo)
{
    return strcasecmp(str, compareTo) == 0;
}

static inline bool ffStrStartsWith(const char* str, const char* compareTo)
{
    return strncmp(str, compareTo, strlen(compareTo)) == 0;
}

static inline bool ffStrEquals(const char* str, const char* compareTo)
{
    return strcmp(str, compareTo) == 0;
}

#endif
