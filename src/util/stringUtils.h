#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

static inline bool ffStrSet(const char* str)
{
    if(str == NULL)
        return false;

    while(isspace(*str))
        str++;

    return *str != '\0';
}


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

static inline bool ffStrContains(const char* str, const char* compareTo)
{
    return strstr(str, compareTo) != NULL;
}

static inline bool ffStrContainsIgnCase(const char* str, const char* compareTo)
{
    return strcasestr(str, compareTo) != NULL;
}

static inline bool ffStrContainsC(const char* str, char compareTo)
{
    return strchr(str, compareTo) != NULL;
}

static inline bool ffCharIsEnglishAlphabet(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static inline bool ffCharIsDigit(char c)
{
    return '0' <= c && c <= '9';
}
