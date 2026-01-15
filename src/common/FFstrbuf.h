#pragma once

#include "FFcheckmacros.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef FF_USE_SYSTEM_YYJSON
    #include <yyjson.h>
#else
    #include "3rdparty/yyjson/yyjson.h"
#endif

#ifdef _WIN32
    // #include <shlwapi.h>
    __stdcall char* StrStrIA(const char* lpFirst, const char* lpSrch);
    #define strcasestr StrStrIA
#endif

#define FASTFETCH_STRBUF_DEFAULT_ALLOC 32

// static string (allocated == 0), chars points to a string literal
// dynamic string (allocated > 0), chars points to a heap allocated buffer
typedef struct FFstrbuf
{
    uint32_t allocated;
    uint32_t length;
    char* chars;
} FFstrbuf;

static inline void ffStrbufInit(FFstrbuf* strbuf);
void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate);
void ffStrbufInitVF(FFstrbuf* strbuf, const char* format, va_list arguments);
void ffStrbufInitMoveNS(FFstrbuf* strbuf, uint32_t length, char* heapStr);

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
void ffStrbufPrependC(FFstrbuf* strbuf, char c);

void ffStrbufInsertNC(FFstrbuf* strbuf, uint32_t index, uint32_t num, char c);

// Clear the content of strbuf and set new value
// NOTE: Unlike ffStrbufAppend*, ffStrbufSet* functions may NOT reserve extra space
void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value);
void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value);
FF_C_PRINTF(2, 3) void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...);

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c);
void ffStrbufTrimRight(FFstrbuf* strbuf, char c);
void ffStrbufTrimLeftSpace(FFstrbuf* strbuf);
void ffStrbufTrimRightSpace(FFstrbuf* strbuf);

bool ffStrbufRemoveSubstr(FFstrbuf* strbuf, uint32_t startIndex, uint32_t endIndex);
void ffStrbufRemoveS(FFstrbuf* strbuf, const char* str);
void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[]);

FF_C_NODISCARD uint32_t ffStrbufNextIndexC(const FFstrbuf* strbuf, uint32_t start, char c);
FF_C_NODISCARD uint32_t ffStrbufNextIndexS(const FFstrbuf* strbuf, uint32_t start, const char* str);

FF_C_NODISCARD uint32_t ffStrbufPreviousIndexC(const FFstrbuf* strbuf, uint32_t start, char c);

void ffStrbufReplaceAllC(FFstrbuf* strbuf, char find, char replace);

// Returns true if the strbuf is modified
bool ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index);
bool ffStrbufSubstrAfter(FFstrbuf* strbuf, uint32_t index); // Not including the index
bool ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, char c);
bool ffStrbufSubstrAfterFirstS(FFstrbuf* strbuf, const char* str);
bool ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, char c);
bool ffStrbufSubstr(FFstrbuf* strbuf, uint32_t start, uint32_t end);

FF_C_NODISCARD uint32_t ffStrbufCountC(const FFstrbuf* strbuf, char c);

bool ffStrbufRemoveIgnCaseEndS(FFstrbuf* strbuf, const char* end);

bool ffStrbufEnsureEndsWithC(FFstrbuf* strbuf, char c);

void ffStrbufWriteTo(const FFstrbuf* strbuf, FILE* file);
void ffStrbufPutTo(const FFstrbuf* strbuf, FILE* file);

FF_C_NODISCARD double ffStrbufToDouble(const FFstrbuf* strbuf, double defaultValue);
FF_C_NODISCARD int64_t ffStrbufToSInt(const FFstrbuf* strbuf, int64_t defaultValue);
FF_C_NODISCARD uint64_t ffStrbufToUInt(const FFstrbuf* strbuf, uint64_t defaultValue);

void ffStrbufUpperCase(FFstrbuf* strbuf);
void ffStrbufLowerCase(FFstrbuf* strbuf);

// Function alters the buffer to extract lines or delimited segments (replaces the delimiter with '\0')
// so that buffer MUST be heap allocated (NOT a static string)
// `lineptr` must be `NULL` and `n` MUST be `0` for the first call
// Caller MUST NOT free `*lineptr`
bool ffStrbufGetdelim(char** lineptr, size_t* n, char delimiter, FFstrbuf* buffer);
void ffStrbufGetdelimRestore(char** lineptr, size_t* n, char delimiter, FFstrbuf* buffer);

/**
 * @brief Read a line from a FFstrbuf.
 *
 * @details Behaves like getline(3) but reads from a FFstrbuf.
 *
 * @param[in,out] lineptr The pointer to a pointer that will be set to the start of the line
                          (points to buffer's internal memory address to avoid memory allocation and copy).
                          MUST NOT be freed by the caller, unlike `getline(3)`.
 *                        MUST be NULL for the first call.
 * @param[in,out] n The pointer to the size of the buffer of lineptr.
                    MUST be 0 for the first call.
 * @param[in] buffer The buffer to read from.
                     MUST be heap allocated (NOT a static string).
 *
 * @return true if a line has been read, false if the end of the buffer has been reached.
 */
static inline bool ffStrbufGetline(char** lineptr, size_t* n, FFstrbuf* buffer)
{
    return ffStrbufGetdelim(lineptr, n, '\n', buffer);
}
/**
 * @brief Restore the end of a line that was modified by ffStrbufGetline.
 * @warning This function should be called before breaking an ffStrbufGetline loop if `buffer` will be used later.
 */
static inline void ffStrbufGetlineRestore(char** lineptr, size_t* n, FFstrbuf* buffer)
{
    ffStrbufGetdelimRestore(lineptr, n, '\n', buffer);
}
bool ffStrbufRemoveDupWhitespaces(FFstrbuf* strbuf);
bool ffStrbufMatchSeparatedNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);
bool ffStrbufMatchSeparatedIgnCaseNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);
bool ffStrbufSeparatedContainNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);
bool ffStrbufSeparatedContainIgnCaseNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);

int ffStrbufAppendUtf32CodePoint(FFstrbuf* strbuf, uint32_t codepoint);

void ffStrbufAppendSInt(FFstrbuf* strbuf, int64_t value);
void ffStrbufAppendUInt(FFstrbuf* strbuf, uint64_t value);
// Appends a double value to the string buffer with the specified precision (0~15).
// if `precision < 0`, let yyjson decide the precision
void ffStrbufAppendDouble(FFstrbuf* strbuf, double value, int8_t precision, bool trailingZeros);

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreateA(uint32_t allocate)
{
    FFstrbuf strbuf;
    ffStrbufInitA(&strbuf, allocate);
    return strbuf;
}

static inline void ffStrbufInitCopy(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict src)
{
    if (src->allocated == 0) // static string
        *strbuf = *src;
    else
    {
        ffStrbufInitA(strbuf, src->allocated);
        ffStrbufAppend(strbuf, src);
    }
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
        *strbuf = *src;
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

static inline void ffStrbufInitMoveS(FFstrbuf* strbuf, char* heapStr)
{
    ffStrbufInitMoveNS(strbuf, (uint32_t) strlen(heapStr), heapStr);
}

static inline void ffStrbufDestroy(FFstrbuf* strbuf)
{
    if(strbuf->allocated > 0)
        free(strbuf->chars);

    ffStrbufInit(strbuf);
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

static inline void ffStrbufSetS(FFstrbuf* strbuf, const char* value)
{
    assert(strbuf != NULL);

    if (value == NULL)
        ffStrbufClear(strbuf);
    else
        ffStrbufSetNS(strbuf, (uint32_t) strlen(value), value);
}

static inline bool ffStrbufSetJsonVal(FFstrbuf* strbuf, yyjson_val* jsonVal)
{
    assert(strbuf != NULL);

    if (yyjson_is_str(jsonVal))
    {
        ffStrbufSetNS(strbuf, (uint32_t) unsafe_yyjson_get_len(jsonVal), unsafe_yyjson_get_str(jsonVal));
        return true;
    }

    ffStrbufClear(strbuf);
    return false;
}

static inline void ffStrbufAppendS(FFstrbuf* strbuf, const char* value)
{
    if(value == NULL)
        return;
    ffStrbufAppendNS(strbuf, (uint32_t) strlen(value), value);
}

static inline bool ffStrbufAppendJsonVal(FFstrbuf* strbuf, yyjson_val* jsonVal)
{
    if (yyjson_is_str(jsonVal))
    {
        ffStrbufAppendNS(strbuf, (uint32_t) unsafe_yyjson_get_len(jsonVal), unsafe_yyjson_get_str(jsonVal));
        return true;
    }
    return false;
}

static inline void ffStrbufInit(FFstrbuf* strbuf)
{
    extern char* CHAR_NULL_PTR;
    strbuf->allocated = strbuf->length = 0;
    strbuf->chars = CHAR_NULL_PTR;
}

FF_C_NODISCARD static inline FFstrbuf ffStrbufCreate(void)
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
        free(strbuf->chars);

    if(value != NULL)
        ffStrbufInitStatic(strbuf, value);
    else
        ffStrbufInit(strbuf);
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

static inline bool ffStrbufInitJsonVal(FFstrbuf* strbuf, yyjson_val* jsonVal)
{
    ffStrbufInit(strbuf);
    return ffStrbufAppendJsonVal(strbuf, jsonVal);
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

static inline FF_C_NODISCARD int ffStrbufComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
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
        return 0;

    return ffStrbufPreviousIndexC(strbuf, strbuf->length - 1, c);
}

static inline bool ffStrbufSubstrBeforeFirstC(FFstrbuf* strbuf, char c)
{
    return ffStrbufSubstrBefore(strbuf, ffStrbufFirstIndexC(strbuf, c));
}

static inline bool ffStrbufSubstrBeforeLastC(FFstrbuf* strbuf, char c)
{
    return ffStrbufSubstrBefore(strbuf, ffStrbufLastIndexC(strbuf, c));
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

static inline FF_C_NODISCARD bool ffStrbufEndsWithFn(const FFstrbuf* strbuf, int (*const fn)(int))
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

static inline void ffStrbufTrimSpace(FFstrbuf* strbuf)
{
    ffStrbufTrimRightSpace(strbuf);
    ffStrbufTrimLeftSpace(strbuf);
}

static inline bool ffStrbufMatchSeparatedS(const FFstrbuf* strbuf, const char* comp, char separator)
{
    return ffStrbufMatchSeparatedNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

static inline bool ffStrbufMatchSeparated(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator)
{
    return ffStrbufMatchSeparatedNS(strbuf, comp->length, comp->chars, separator);
}

static inline bool ffStrbufMatchSeparatedIgnCaseS(const FFstrbuf* strbuf, const char* comp, char separator)
{
    return ffStrbufMatchSeparatedIgnCaseNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

static inline bool ffStrbufMatchSeparatedIgnCase(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator)
{
    return ffStrbufMatchSeparatedIgnCaseNS(strbuf, comp->length, comp->chars, separator);
}

static inline bool ffStrbufSeparatedContainS(const FFstrbuf* strbuf, const char* comp, char separator)
{
    return ffStrbufSeparatedContainNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

static inline bool ffStrbufSeparatedContain(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator)
{
    return ffStrbufSeparatedContainNS(strbuf, comp->length, comp->chars, separator);
}

static inline bool ffStrbufSeparatedContainIgnCaseS(const FFstrbuf* strbuf, const char* comp, char separator)
{
    return ffStrbufSeparatedContainIgnCaseNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

static inline bool ffStrbufSeparatedContainIgnCase(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator)
{
    return ffStrbufSeparatedContainIgnCaseNS(strbuf, comp->length, comp->chars, separator);
}

#define FF_STRBUF_AUTO_DESTROY FFstrbuf __attribute__((__cleanup__(ffStrbufDestroy)))
