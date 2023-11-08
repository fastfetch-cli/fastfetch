#pragma once

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

static inline bool ffStrEndsWith(const char* str, const char* compareTo)
{
    size_t strLength = strlen(str);
    size_t compareToLength = strlen(compareTo);
    if (strLength < compareToLength)
        return false;
    return memcmp(str + strLength - compareToLength, compareTo, compareToLength) == 0;
}

static inline bool ffStrEndsWithIgnCase(const char* str, const char* compareTo)
{
    size_t strLength = strlen(str);
    size_t compareToLength = strlen(compareTo);
    if (strLength < compareToLength)
        return false;
    return strncasecmp(str + strLength - compareToLength, compareTo, compareToLength) == 0;
}

static inline bool ffStrEquals(const char* str, const char* compareTo)
{
    return strcmp(str, compareTo) == 0;
}
