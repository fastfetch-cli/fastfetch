#pragma once

#ifndef FASTFETCH_INCLUDED_FFSTRBUF
#define FASTFETCH_INCLUDED_FFSTRBUF

#include "FFcheckmacros.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
    // #include <shlwapi.h>
    __stdcall const char* StrStrIA(const char* lpFirst, const char* lpSrch);
    #define strcasestr StrStrIA
#endif

#define FASTFETCH_STRBUF_DEFAULT_ALLOC 32

typedef struct FFstrbuf
{
    uint32_t allocated;
    uint32_t length;
    char* chars;
} FFstrbuf;

static inline void ffStrbufInit(FFstrbuf* strbuf);
void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate);
void ffStrbufInitVF(FFstrbuf* strbuf, const char* format, va_list arguments);

void ffStrbufEnsureFree(FFstrbuf* strbuf, uint32_t free);
void ffStrbufEnsureFixedLengthFree(FFstrbuf* strbuf, uint32_t free);

void ffStrbufClear(FFstrbuf* strbuf);

static inline void ffStrbufAppend(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict value);
void ffStrbufAppendC(FFstrbuf* strbuf, char c);
void ffStrbufAppendNC(FFstrbuf* strbuf, uint32_t num, char c);
void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufAppendTransformS(FFstrbuf* strbuf, const char* value, int(*transformFunc)(int));
FF_C_PRINTF(2, 3) void ffStrbufAppendF(FFstrbuf* strbuf, const char* format, ...);
void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments);
const char* ffStrbufAppendSUntilC(FFstrbuf* strbuf, const char* value, char until);

void ffStrbufPrependNS(FFstrbuf* strbuf, uint32_t length, const char* value);

void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value);
FF_C_PRINTF(2, 3) void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...);

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c);
void ffStrbufTrimRight(FFstrbuf* strbuf, char c);
void ffStrbufTrimRightSpace(FFstrbuf* strbuf);

void ffStrbufRemoveSubstr(FFstrbuf* strbuf, uint32_t startIndex, uint32_t endIndex);
void ffStrbufRemoveS(FFstrbuf* strbuf, const char* str);
void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[]);

FF_C_NODISCARD uint32_t ffStrbufNextIndexC(const FFstrbuf* strbuf, uint32_t start, char c);
FF_C_NODISCARD uint32_t ffStrbufNextIndexS(const FFstrbuf* strbuf, uint32_t start, const char* str);

FF_C_NODISCARD uint32_t ffStrbufPreviousIndexC(const FFstrbuf* strbuf, uint32_t start, char c);

void ffStrbufReplaceAllC(FFstrbuf* strbuf, char find, char replace);

void ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index);
void ffStrbufSubstrAfter(FFstrbuf* strbuf, uint32_t index); // Not including the index
void ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, char c);
void ffStrbufSubstrAfterFirstS(FFstrbuf* strbuf, const char* str);
void ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, char c);

FF_C_NODISCARD uint32_t ffStrbufCountC(const FFstrbuf* strbuf, char c);

bool ffStrbufRemoveIgnCaseEndS(FFstrbuf* strbuf, const char* end);

void ffStrbufEnsureEndsWithC(FFstrbuf* strbuf, char c);

void ffStrbufWriteTo(const FFstrbuf* strbuf, FILE* file);
void ffStrbufPutTo(const FFstrbuf* strbuf, FILE* file);

FF_C_NODISCARD double ffStrbufToDouble(const FFstrbuf* strbuf);
FF_C_NODISCARD int64_t ffStrbufToSInt(const FFstrbuf* strbuf, int64_t defaultValue);
FF_C_NODISCARD uint64_t ffStrbufToUInt(const FFstrbuf* strbuf, uint64_t defaultValue);

void ffStrbufUpperCase(FFstrbuf* strbuf);
void ffStrbufLowerCase(FFstrbuf* strbuf);

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateA(uint32_t allocate)
{
    FFstrbuf strbuf;
    ffStrbufInitA(&strbuf, allocate);
    return strbuf;
}

static inline void ffStrbufInitCopy(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict src)
{
    ffStrbufInitA(strbuf, src->allocated);
    ffStrbufAppend(strbuf, src);
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateCopy(const FFstrbuf* src)
{
    FFstrbuf strbuf;
    ffStrbufInitCopy(&strbuf, src);
    return strbuf;
}

// Move the content of `src` into `strbuf`, and left `src` empty
static inline void ffStrbufInitMove(FFstrbuf* strbuf, FFstrbuf* src)
{
    if (src)
    {
        strbuf->allocated = src->allocated;
        strbuf->chars = src->chars;
        strbuf->length = src->length;
        ffStrbufInit(src);
    }
    else
        ffStrbufInit(strbuf);
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateMove(FFstrbuf* src)
{
    FFstrbuf strbuf;
    ffStrbufInitMove(&strbuf, src);
    return strbuf;
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateVF(const char* format, va_list arguments)
{
    FFstrbuf strbuf;
    ffStrbufInitVF(&strbuf, format, arguments);
    return strbuf;
}

FF_C_PRINTF(2, 3)
static inline void ffStrbufInitF(FFstrbuf* strbuf, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    ffStrbufInitVF(strbuf, format, arguments);
    va_end(arguments);
}

FF_C_PRINTF(1, 2)
FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateF(const char* format, ...)
{
    FFstrbuf strbuf;

    va_list arguments;
    va_start(arguments, format);
    ffStrbufInitVF(&strbuf, format, arguments);
    va_end(arguments);

    return strbuf;
}

static inline void ffStrbufDestroy(FFstrbuf* strbuf)
{
    extern char* CHAR_NULL_PTR;

    if(strbuf->allocated == 0)
    {
        strbuf->length = 0;
        strbuf->chars = CHAR_NULL_PTR;
        return;
    }

    //Avoid free-after-use. These 3 assignments are cheap so don't remove them
    strbuf->allocated = strbuf->length = 0;
    free(strbuf->chars);
    strbuf->chars = CHAR_NULL_PTR;
}

FF_C_NODISCARD static inline uint32_t ffStrbufGetFree(const FFstrbuf* strbuf)
{
    assert(strbuf != NULL);
    if(strbuf->allocated == 0)
        return 0;

    return strbuf->allocated - strbuf->length - 1; // - 1 for the null byte
}

static inline void ffStrbufRecalculateLength(FFstrbuf* strbuf)
{
    strbuf->length = (uint32_t) strlen(strbuf->chars);
}

static inline void ffStrbufAppendS(FFstrbuf* strbuf, const char* value)
{
    if(value == NULL)
        return;
    ffStrbufAppendNS(strbuf, (uint32_t) strlen(value), value);
}

static inline void ffStrbufSetS(FFstrbuf* strbuf, const char* value)
{
    ffStrbufClear(strbuf);

    if(value != NULL)
        ffStrbufAppendNS(strbuf, (uint32_t) strlen(value), value);
}

static inline void ffStrbufInit(FFstrbuf* strbuf)
{
    extern char* CHAR_NULL_PTR;
    strbuf->allocated = 0;
    strbuf->length = 0;
    strbuf->chars = CHAR_NULL_PTR;
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreate()
{
    FFstrbuf strbuf;
    ffStrbufInit(&strbuf);
    return strbuf;
}

static inline void ffStrbufInitStatic(FFstrbuf* strbuf, const char* str)
{
    ffStrbufInit(strbuf);
    if (!str) return;

    strbuf->allocated = 0;
    strbuf->length = (uint32_t) strlen(str);
    strbuf->chars = (char*) str;
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateStatic(const char* str)
{
    FFstrbuf strbuf;
    ffStrbufInitStatic(&strbuf, str);
    return strbuf;
}

static inline void ffStrbufSetStatic(FFstrbuf* strbuf, const char* value)
{
    if(strbuf->allocated > 0)
        ffStrbufDestroy(strbuf);

    if(value != NULL)
        ffStrbufInitStatic(strbuf, value);
}

static inline void ffStrbufInitNS(FFstrbuf* strbuf, uint32_t length, const char* str)
{
    ffStrbufInit(strbuf);
    ffStrbufAppendNS(strbuf, length, str);
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateNS(uint32_t length, const char* str)
{
    FFstrbuf strbuf;
    ffStrbufInitNS(&strbuf, length, str);
    return strbuf;
}

static inline void ffStrbufInitS(FFstrbuf* strbuf, const char* str)
{
    ffStrbufInit(strbuf);
    ffStrbufAppendS(strbuf, str);
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateS(const char* str)
{
    FFstrbuf strbuf;
    ffStrbufInitS(&strbuf, str);
    return strbuf;
}

static inline void ffStrbufAppend(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict value)
{
    assert(value != strbuf);
    if(value == NULL)
        return;
    ffStrbufAppendNS(strbuf, value->length, value->chars);
}

static inline void ffStrbufPrepend(FFstrbuf* strbuf, FFstrbuf* value)
{
    if(value == NULL)
        return;
    ffStrbufPrependNS(strbuf, value->length, value->chars);
}

static inline void ffStrbufPrependS(FFstrbuf* strbuf, const char* value)
{
    if(value == NULL)
        return;
    ffStrbufPrependNS(strbuf, (uint32_t) strlen(value), value);
}

static inline int ffStrbufComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    uint32_t length = strbuf->length > comp->length ? comp->length : strbuf->length;
    return memcmp(strbuf->chars, comp->chars, length + 1);
}

static inline FF_C_NODISCARD bool ffStrbufEqual(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    return ffStrbufComp(strbuf, comp) == 0;
}

static inline FF_C_NODISCARD int ffStrbufCompS(const FFstrbuf* strbuf, const char* comp)
{
    return strcmp(strbuf->chars, comp);
}

static inline FF_C_NODISCARD bool ffStrbufEqualS(const FFstrbuf* strbuf, const char* comp)
{
    return ffStrbufCompS(strbuf, comp) == 0;
}

static inline FF_C_NODISCARD int ffStrbufIgnCaseCompS(const FFstrbuf* strbuf, const char* comp)
{
    return strcasecmp(strbuf->chars, comp);
}

static inline FF_C_NODISCARD bool ffStrbufIgnCaseEqualS(const FFstrbuf* strbuf, const char* comp)
{
    return ffStrbufIgnCaseCompS(strbuf, comp) == 0;
}

static inline FF_C_NODISCARD int ffStrbufIgnCaseComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    return ffStrbufIgnCaseCompS(strbuf, comp->chars);
}

static inline FF_C_NODISCARD bool ffStrbufIgnCaseEqual(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    return ffStrbufIgnCaseComp(strbuf, comp) == 0;
}

static inline FF_C_NODISCARD bool ffStrbufContainC(const FFstrbuf* strbuf, char c)
{
    return memchr(strbuf->chars, c, strbuf->length) != NULL;
}

static inline FF_C_NODISCARD bool ffStrbufContainS(const FFstrbuf* strbuf, const char* str)
{
    return strstr(strbuf->chars, str) != NULL;
}

static inline FF_C_NODISCARD bool ffStrbufContain(const FFstrbuf* strbuf, const FFstrbuf* str)
{
    return ffStrbufContainS(strbuf, str->chars);
}

static inline FF_C_NODISCARD bool ffStrbufContainIgnCaseS(const FFstrbuf* strbuf, const char* str)
{
    return strcasestr(strbuf->chars, str) != NULL;
}

static inline FF_C_NODISCARD bool ffStrbufContainIgnCase(const FFstrbuf* strbuf, const FFstrbuf* str)
{
    return ffStrbufContainIgnCaseS(strbuf, str->chars);
}

static inline FF_C_NODISCARD uint32_t ffStrbufFirstIndexC(const FFstrbuf* strbuf, char c)
{
    return ffStrbufNextIndexC(strbuf, 0, c);
}

static inline FF_C_NODISCARD uint32_t ffStrbufFirstIndex(const FFstrbuf* strbuf, const FFstrbuf* searched)
{
    return ffStrbufNextIndexS(strbuf, 0, searched->chars);
}

static inline FF_C_NODISCARD uint32_t ffStrbufFirstIndexS(const FFstrbuf* strbuf, const char* str)
{
    return ffStrbufNextIndexS(strbuf, 0, str);
}

static inline FF_C_NODISCARD uint32_t ffStrbufLastIndexC(const FFstrbuf* strbuf, char c)
{
    if(strbuf->length == 0)
        return strbuf->length;

    return ffStrbufPreviousIndexC(strbuf, strbuf->length - 1, c);
}

static inline void ffStrbufSubstrBeforeFirstC(FFstrbuf* strbuf, char c)
{
    ffStrbufSubstrBefore(strbuf, ffStrbufFirstIndexC(strbuf, c));
}

static inline void ffStrbufSubstrBeforeLastC(FFstrbuf* strbuf, char c)
{
    ffStrbufSubstrBefore(strbuf, ffStrbufLastIndexC(strbuf, c));
}

static inline FF_C_NODISCARD bool ffStrbufStartsWithC(const FFstrbuf* strbuf, char c)
{
    return strbuf->chars[0] == c;
}

static inline FF_C_NODISCARD bool ffStrbufStartsWithSN(const FFstrbuf* strbuf, const char* start, uint32_t length)
{
    if (length > strbuf->length)
        return false;

    return memcmp(strbuf->chars, start, length) == 0;
}

static inline FF_C_NODISCARD bool ffStrbufStartsWithS(const FFstrbuf* strbuf, const char* start)
{
    return ffStrbufStartsWithSN(strbuf, start, (uint32_t) strlen(start));
}

static inline FF_C_NODISCARD bool ffStrbufStartsWith(const FFstrbuf* strbuf, const FFstrbuf* start)
{
    return ffStrbufStartsWithSN(strbuf, start->chars, start->length);
}

static inline FF_C_NODISCARD bool ffStrbufStartsWithIgnCaseNS(const FFstrbuf* strbuf, uint32_t length, const char* start)
{
    if(length > strbuf->length)
        return false;
    return strncasecmp(strbuf->chars, start, length) == 0;
}

static inline FF_C_NODISCARD bool ffStrbufStartsWithIgnCaseS(const FFstrbuf* strbuf, const char* start)
{
    return ffStrbufStartsWithIgnCaseNS(strbuf, (uint32_t) strlen(start), start);
}

static inline FF_C_NODISCARD bool ffStrbufStartsWithIgnCase(const FFstrbuf* strbuf, const FFstrbuf* start)
{
    return ffStrbufStartsWithIgnCaseNS(strbuf, start->length, start->chars);
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithC(const FFstrbuf* strbuf, char c)
{
    return strbuf->length == 0 ? false :
        strbuf->chars[strbuf->length - 1] == c;
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithNS(const FFstrbuf* strbuf, uint32_t endLength, const char* end)
{
    if(endLength > strbuf->length)
        return false;

    return memcmp(strbuf->chars + strbuf->length - endLength, end, endLength) == 0;
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithS(const FFstrbuf* strbuf, const char* end)
{
    return ffStrbufEndsWithNS(strbuf, (uint32_t) strlen(end), end);
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithFn(const FFstrbuf* strbuf, int (*fn)(int))
{
    return strbuf->length == 0 ? false :
        fn(strbuf->chars[strbuf->length - 1]);
}

static inline FF_C_NODISCARD bool ffStrbufEndsWith(const FFstrbuf* strbuf, const FFstrbuf* end)
{
    return ffStrbufEndsWithNS(strbuf, end->length, end->chars);
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithIgnCaseNS(const FFstrbuf* strbuf, uint32_t endLength, const char* end)
{
    if(endLength > strbuf->length)
        return false;
    return strcasecmp(strbuf->chars + strbuf->length - endLength, end) == 0;
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithIgnCaseS(const FFstrbuf* strbuf, const char* end)
{
    return ffStrbufEndsWithIgnCaseNS(strbuf, (uint32_t) strlen(end), end);
}

static inline FF_C_NODISCARD bool ffStrbufEndsWithIgnCase(const FFstrbuf* strbuf, const FFstrbuf* end)
{
    return ffStrbufEndsWithIgnCaseNS(strbuf, end->length, end->chars);
}

static inline void ffStrbufTrim(FFstrbuf* strbuf, char c)
{
    ffStrbufTrimRight(strbuf, c);
    ffStrbufTrimLeft(strbuf, c);
}

#define FF_STRBUF_AUTO_DESTROY FFstrbuf __attribute__((__cleanup__(ffStrbufDestroy)))

#endif
