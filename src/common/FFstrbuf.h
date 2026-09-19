#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "common/memrchr.h"
#include "common/arrutil.h"

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

// ---------------------------------------------------------------------------------------------
// Contract attributes used throughout this header:
//
// `gnu::pure` marks a function that reads memory (through its arguments and through globals) but
// writes nothing observable, so the compiler may cache and reorder calls to it. It is applied to
// the comparison / search accessors, and deliberately NOT to:
//   * the `To*` converters -- `strtod` / `strtoull` / `strtoll` write `errno`;
//   * `ffStrbufEndsWithFn` -- the caller-supplied `fn` may have side effects;
//   * `ffStrbufWriteTo` / `ffStrbufPutTo` -- they write to a `FILE*`.
//
// `gnu::nonnull(N)` is added wherever the function dereferences argument N unconditionally. The
// two cannot be combined with a null check: clang reports `-Wtautological-pointer-compare` /
// `-Wpointer-bool-conversion` (both in `-Wall`) when a `nonnull` parameter is compared against
// null, so a redundant `assert(p != nullptr)` is dropped in favour of the attribute. Asserts that
// state something `nonnull` cannot express -- pointer aliasing (`value != strbuf`), index ranges
// (`start <= strbuf->length`), or a non-pointer invariant -- are kept.
//
// `nodiscard` is NOT applied to the "was the buffer modified" boolean returns
// (`ffStrbufSubstr*`, `ffStrbufRemoveSubstr`, `ffStrbufEnsureEndsWithC`, ...). Ignoring that
// result is a normal, intended use of those functions -- `ffStrbufSubstrBefore` alone has 153 call
// sites and ~147 of them discard the result -- so `nodiscard` would only produce noise.
// ---------------------------------------------------------------------------------------------

// static string (allocated == 0), chars points to a string literal
// dynamic string (allocated > 0), chars points to a heap allocated buffer
typedef struct FFstrbuf {
    uint32_t allocated;
    uint32_t length;
    char* chars;
} FFstrbuf;

[[gnu::nonnull(1)]] static inline void ffStrbufInit(FFstrbuf* strbuf);
[[gnu::nonnull(1)]] void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate);
[[gnu::nonnull(1, 2), gnu::format(printf, 2, 0)]] void ffStrbufInitVF(FFstrbuf* strbuf, const char* format, va_list arguments);
[[gnu::nonnull(1, 3)]] void ffStrbufInitMoveNS(FFstrbuf* strbuf, uint32_t length, char* heapStr);
[[gnu::format(printf, 2, 3), gnu::nonnull(1, 2)]] void ffStrbufInitF(FFstrbuf* strbuf, const char* format, ...);
[[gnu::format(printf, 1, 2), gnu::nonnull(1), nodiscard]] FFstrbuf ffStrbufCreateF(const char* format, ...);

[[gnu::nonnull(1)]] void ffStrbufEnsureFixedLengthFree(FFstrbuf* strbuf, uint32_t free);
[[gnu::nonnull(1)]] void ffStrbufEnsureFreeNoCheck(FFstrbuf* strbuf, uint32_t free);

[[gnu::nonnull(1)]] static inline void ffStrbufAppend(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict value);
[[gnu::nonnull(1, 3)]] void ffStrbufAppendTransformS(FFstrbuf* strbuf, const char* value, int (*transformFunc)(int));
[[gnu::format(printf, 2, 3), gnu::nonnull(1, 2)]] void ffStrbufAppendF(FFstrbuf* strbuf, const char* format, ...);
[[gnu::nonnull(1, 2), gnu::format(printf, 2, 0)]] void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments);
// Returns the pointer to the terminator, or nullptr if `value` was nullptr; callers are free to
// ignore it, so not `nodiscard`.
[[gnu::nonnull(1)]] const char* ffStrbufAppendSUntilC(FFstrbuf* strbuf, const char* value, char until);

[[gnu::nonnull(1)]] void ffStrbufPrependNS(FFstrbuf* strbuf, uint32_t length, const char* value);
[[gnu::nonnull(1)]] void ffStrbufPrependC(FFstrbuf* strbuf, char c);

[[gnu::nonnull(1)]] void ffStrbufInsertNC(FFstrbuf* strbuf, uint32_t index, uint32_t num, char c);

// Clear the content of strbuf and set new value
// NOTE: Unlike ffStrbufAppend*, ffStrbufSet* functions may NOT reserve extra space
[[gnu::nonnull(1, 2)]] void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value);
[[gnu::nonnull(1, 3)]] void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value);
[[gnu::format(printf, 2, 3), gnu::nonnull(1, 2)]] void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...);

[[gnu::nonnull(1)]] void ffStrbufTrimLeft(FFstrbuf* strbuf, char c);
[[gnu::nonnull(1)]] void ffStrbufTrimRight(FFstrbuf* strbuf, char c);
[[gnu::nonnull(1)]] void ffStrbufTrimLeftSpace(FFstrbuf* strbuf);
[[gnu::nonnull(1)]] void ffStrbufTrimRightSpace(FFstrbuf* strbuf);

[[gnu::nonnull(1)]] bool ffStrbufRemoveSubstr(FFstrbuf* strbuf, uint32_t startIndex, uint32_t endIndex);
[[gnu::nonnull(1, 2)]] void ffStrbufRemoveS(FFstrbuf* strbuf, const char* str);
// `strings` is only dereferenced when `numStrings > 0`, so it is intentionally not `nonnull(3)`
[[gnu::nonnull(1)]] void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[]);

[[gnu::nonnull(1)]] void ffStrbufReplaceAllC(FFstrbuf* strbuf, char find, char replace);

// Returns true if the strbuf is modified
[[gnu::nonnull(1)]] bool ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index);
[[gnu::nonnull(1)]] bool ffStrbufSubstrAfter(FFstrbuf* strbuf, uint32_t index); // Not including the index
[[gnu::nonnull(1)]] bool ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, char c);
[[gnu::nonnull(1, 2)]] bool ffStrbufSubstrAfterFirstS(FFstrbuf* strbuf, const char* str);
[[gnu::nonnull(1)]] bool ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, char c);
[[gnu::nonnull(1)]] bool ffStrbufSubstr(FFstrbuf* strbuf, uint32_t start, uint32_t end);

[[gnu::nonnull(1), gnu::pure, nodiscard]] uint32_t ffStrbufCountC(const FFstrbuf* strbuf, char c);

[[gnu::nonnull(1, 2)]] bool ffStrbufRemoveIgnCaseEndS(FFstrbuf* strbuf, const char* end);

[[gnu::nonnull(1)]] bool ffStrbufEnsureEndsWithC(FFstrbuf* strbuf, char c);

[[gnu::nonnull(1)]] void ffStrbufUpperCase(FFstrbuf* strbuf);
[[gnu::nonnull(1)]] void ffStrbufLowerCase(FFstrbuf* strbuf);

// Function alters the buffer to extract lines or delimited segments (replaces the delimiter with '\0')
// so that buffer MUST be heap allocated (NOT a static string)
// `lineptr` must be `nullptr` and `n` MUST be `0` for the first call
// Caller MUST NOT free `*lineptr`
[[gnu::nonnull(1, 2, 4)]] bool ffStrbufGetdelim(char** lineptr, size_t* n, char delimiter, FFstrbuf* buffer);
[[gnu::nonnull(1, 2, 4)]] void ffStrbufGetdelimRestore(char** lineptr, size_t* n, char delimiter, FFstrbuf* buffer);

/**
 * @brief Read a line from a FFstrbuf.
 *
 * @details Behaves like getline(3) but reads from a FFstrbuf.
 *
 * @param[in,out] lineptr The pointer to a pointer that will be set to the start of the line
                          (points to buffer's internal memory address to avoid memory allocation and copy).
                          MUST NOT be freed by the caller, unlike `getline(3)`.
 *                        MUST be nullptr for the first call.
 * @param[in,out] n The pointer to the size of the buffer of lineptr.
                    MUST be 0 for the first call.
 * @param[in] buffer The buffer to read from.
                     MUST be heap allocated (NOT a static string).
 *
 * @return true if a line has been read, false if the end of the buffer has been reached.
 */
[[gnu::nonnull(1, 2, 3), nodiscard]] static inline bool ffStrbufGetline(char** lineptr, size_t* n, FFstrbuf* buffer) {
    return ffStrbufGetdelim(lineptr, n, '\n', buffer);
}
/**
 * @brief Restore the end of a line that was modified by ffStrbufGetline.
 * @warning This function should be called before breaking an ffStrbufGetline loop if `buffer` will be used later.
 */
[[gnu::nonnull(1, 2, 3)]] static inline void ffStrbufGetlineRestore(char** lineptr, size_t* n, FFstrbuf* buffer) {
    ffStrbufGetdelimRestore(lineptr, n, '\n', buffer);
}
[[gnu::nonnull(1)]] bool ffStrbufRemoveDupWhitespaces(FFstrbuf* strbuf);
// `comp` is only dereferenced when `compLength > 0` in the first pair, and is never dereferenced
// when `strbuf` is empty in the second pair, so neither takes `nonnull(3)`
[[gnu::nonnull(1), gnu::pure, nodiscard]] bool ffStrbufMatchSeparatedNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);
[[gnu::nonnull(1), gnu::pure, nodiscard]] bool ffStrbufMatchSeparatedIgnCaseNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);
[[gnu::nonnull(1), gnu::pure, nodiscard]] bool ffStrbufSeparatedContainNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);
[[gnu::nonnull(1), gnu::pure, nodiscard]] bool ffStrbufSeparatedContainIgnCaseNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator);

[[gnu::nonnull(1)]] int ffStrbufAppendUtf32CodePoint(FFstrbuf* strbuf, uint32_t codepoint);

[[gnu::nonnull(1)]] void ffStrbufAppendSInt(FFstrbuf* strbuf, int64_t value);
[[gnu::nonnull(1)]] void ffStrbufAppendUInt(FFstrbuf* strbuf, uint64_t value);
// Appends a double value to the string buffer with the specified precision (0~15).
// if `precision < 0`, let yyjson decide the precision
[[gnu::nonnull(1)]] void ffStrbufAppendDouble(FFstrbuf* strbuf, double value, int8_t precision, bool trailingZeros);

[[nodiscard]] static inline FFstrbuf ffStrbufCreateA(uint32_t allocate) {
    FFstrbuf strbuf;
    ffStrbufInitA(&strbuf, allocate);
    return strbuf;
}

[[gnu::nonnull(1, 2)]] static inline void ffStrbufInitCopy(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict src) {
    if (src->allocated == 0) { // static string
        *strbuf = *src;
    } else {
        ffStrbufInitA(strbuf, src->allocated);
        ffStrbufAppend(strbuf, src);
    }
}

[[gnu::nonnull(1), nodiscard]] static inline FFstrbuf ffStrbufCreateCopy(const FFstrbuf* src) {
    FFstrbuf strbuf;
    ffStrbufInitCopy(&strbuf, src);
    return strbuf;
}

// Move the content of `src` into `strbuf`, and left `src` empty
[[gnu::nonnull(1)]] static inline void ffStrbufInitMove(FFstrbuf* strbuf, FFstrbuf* src) {
    if (src) {
        *strbuf = *src;
        ffStrbufInit(src);
    } else {
        ffStrbufInit(strbuf);
    }
}

[[nodiscard]] static inline FFstrbuf ffStrbufCreateMove(FFstrbuf* src) {
    FFstrbuf strbuf;
    ffStrbufInitMove(&strbuf, src);
    return strbuf;
}

[[gnu::nonnull(1, 2)]] static inline void ffStrbufInitMoveS(FFstrbuf* strbuf, char* heapStr) {
    ffStrbufInitMoveNS(strbuf, (uint32_t) strlen(heapStr), heapStr);
}

// Despite the name, this function resets strbuf to the initial/unallocated state
[[gnu::nonnull(1)]] static inline void ffStrbufDestroy(FFstrbuf* strbuf) {
    if (strbuf->allocated > 0) {
        free(strbuf->chars);
    }

    ffStrbufInit(strbuf);
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline uint32_t ffStrbufGetFree(const FFstrbuf* strbuf) {
    if (strbuf->allocated == 0) {
        return 0;
    }

    return strbuf->allocated - strbuf->length - 1; // - 1 for the null byte
}

[[gnu::nonnull(1)]] static inline void ffStrbufEnsureFree(FFstrbuf* strbuf, uint32_t free) {
    if (__builtin_expect(free == 0, false)) {
        if (__builtin_expect(!(strbuf->allocated == 0 && strbuf->length > 0), true)) {
            return;
        }
    } else {
        if (__builtin_expect(ffStrbufGetFree(strbuf) >= free, true)) {
            return;
        }
    }

    ffStrbufEnsureFreeNoCheck(strbuf, free);
}


[[gnu::nonnull(1)]] static inline void ffStrbufClear(FFstrbuf* strbuf) {
    extern char* CHAR_NULL_PTR;

    if (strbuf->allocated == 0) {
        strbuf->chars = CHAR_NULL_PTR;
    } else {
        strbuf->chars[0] = '\0';
    }

    strbuf->length = 0;
}

[[gnu::nonnull(1)]] static inline void ffStrbufAppendC(FFstrbuf* strbuf, char c) {
    ffStrbufEnsureFree(strbuf, 1);
    strbuf->chars[strbuf->length++] = c;
    strbuf->chars[strbuf->length] = '\0';
}

[[gnu::nonnull(1)]] static inline void ffStrbufAppendNC(FFstrbuf* strbuf, uint32_t num, char c) {
    if (__builtin_expect(num == 0, false)) {
        return;
    }
    ffStrbufEnsureFree(strbuf, num);

    memset(&strbuf->chars[strbuf->length], c, num);
    strbuf->length += num;
    strbuf->chars[strbuf->length] = '\0';
}

[[gnu::nonnull(1)]] static inline void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value) {
    if (__builtin_expect(value == nullptr || length == 0, false)) {
        return;
    }
    ffStrbufEnsureFree(strbuf, length);

    memcpy(&strbuf->chars[strbuf->length], value, length);
    strbuf->length += length;
    strbuf->chars[strbuf->length] = '\0';
}

[[gnu::nonnull(1)]] static inline void ffStrbufAppend(FFstrbuf* __restrict strbuf, const FFstrbuf* __restrict value) {
    assert(value != strbuf);
    if (value == nullptr) {
        return;
    }
    ffStrbufAppendNS(strbuf, value->length, value->chars);
}

[[gnu::nonnull(1)]] static inline void ffStrbufRecalculateLength(FFstrbuf* strbuf) {
    strbuf->length = (uint32_t) strlen(strbuf->chars);
}

// `value` may be null (clears the buffer); `strbuf` may not
[[gnu::nonnull(1)]] static inline void ffStrbufSetS(FFstrbuf* strbuf, const char* value) {
    if (value == nullptr) {
        ffStrbufClear(strbuf);
    } else {
        ffStrbufSetNS(strbuf, (uint32_t) strlen(value), value);
    }
}

[[gnu::nonnull(1)]] static inline bool ffStrbufSetJsonVal(FFstrbuf* strbuf, yyjson_val* jsonVal) {
    if (yyjson_is_str(jsonVal)) {
        ffStrbufSetNS(strbuf, (uint32_t) unsafe_yyjson_get_len(jsonVal), unsafe_yyjson_get_str(jsonVal));
        return true;
    }

    ffStrbufClear(strbuf);
    return false;
}

[[gnu::nonnull(1)]] static inline void ffStrbufAppendS(FFstrbuf* strbuf, const char* value) {
    if (value == nullptr) {
        return;
    }
    ffStrbufAppendNS(strbuf, (uint32_t) strlen(value), value);
}

// Returns whether `jsonVal` was a string. Callers routinely pre-check with `yyjson_is_str`, so the
// result is not `nodiscard`.
static inline bool ffStrbufAppendJsonVal(FFstrbuf* strbuf, yyjson_val* jsonVal) {
    if (yyjson_is_str(jsonVal)) {
        ffStrbufAppendNS(strbuf, (uint32_t) unsafe_yyjson_get_len(jsonVal), unsafe_yyjson_get_str(jsonVal));
        return true;
    }
    return false;
}

[[gnu::nonnull(1)]] static inline void ffStrbufInit(FFstrbuf* strbuf) {
    extern char* CHAR_NULL_PTR;
    strbuf->allocated = strbuf->length = 0;
    strbuf->chars = CHAR_NULL_PTR;
}

[[nodiscard]] static inline FFstrbuf ffStrbufCreate(void) {
    FFstrbuf strbuf;
    ffStrbufInit(&strbuf);
    return strbuf;
}

static inline void ffStrbufInitStatic(FFstrbuf* strbuf, const char* str) {
    ffStrbufInit(strbuf);
    if (!str) {
        return;
    }

    strbuf->allocated = 0;
    strbuf->length = (uint32_t) strlen(str);
    strbuf->chars = (char*) str;
}

[[nodiscard]] static inline FFstrbuf ffStrbufCreateStatic(const char* str) {
    FFstrbuf strbuf;
    ffStrbufInitStatic(&strbuf, str);
    return strbuf;
}

[[gnu::nonnull(1)]] static inline void ffStrbufSetStatic(FFstrbuf* strbuf, const char* value) {
    if (strbuf->allocated > 0) {
        free(strbuf->chars);
    }

    if (value != nullptr) {
        ffStrbufInitStatic(strbuf, value);
    } else {
        ffStrbufInit(strbuf);
    }
}

[[gnu::nonnull(1)]] static inline void ffStrbufInitNS(FFstrbuf* strbuf, uint32_t length, const char* str) {
    ffStrbufInit(strbuf);
    ffStrbufAppendNS(strbuf, length, str);
}

[[nodiscard]] static inline FFstrbuf ffStrbufCreateNS(uint32_t length, const char* str) {
    FFstrbuf strbuf;
    ffStrbufInitNS(&strbuf, length, str);
    return strbuf;
}

// Returns whether `jsonVal` was a string; not `nodiscard`, same reason as ffStrbufAppendJsonVal
static inline bool ffStrbufInitJsonVal(FFstrbuf* strbuf, yyjson_val* jsonVal) {
    ffStrbufInit(strbuf);
    return ffStrbufAppendJsonVal(strbuf, jsonVal);
}

[[gnu::nonnull(1)]] static inline void ffStrbufInitS(FFstrbuf* strbuf, const char* str) {
    ffStrbufInit(strbuf);
    ffStrbufAppendS(strbuf, str);
}

[[nodiscard]] static inline FFstrbuf ffStrbufCreateS(const char* str) {
    FFstrbuf strbuf;
    ffStrbufInitS(&strbuf, str);
    return strbuf;
}

[[gnu::nonnull(1)]] static inline void ffStrbufPrepend(FFstrbuf* strbuf, FFstrbuf* value) {
    if (value == nullptr) {
        return;
    }
    ffStrbufPrependNS(strbuf, value->length, value->chars);
}

[[gnu::nonnull(1)]] static inline void ffStrbufPrependS(FFstrbuf* strbuf, const char* value) {
    if (value == nullptr) {
        return;
    }
    ffStrbufPrependNS(strbuf, (uint32_t) strlen(value), value);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline int ffStrbufComp(const FFstrbuf* strbuf, const FFstrbuf* comp) {
    uint32_t length = strbuf->length > comp->length ? comp->length : strbuf->length;
    return memcmp(strbuf->chars, comp->chars, length + 1);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufEqual(const FFstrbuf* strbuf, const FFstrbuf* comp) {
    return ffStrbufComp(strbuf, comp) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline int ffStrbufCompS(const FFstrbuf* strbuf, const char* comp) {
    return strcmp(strbuf->chars, comp);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufEqualS(const FFstrbuf* strbuf, const char* comp) {
    return ffStrbufCompS(strbuf, comp) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline int ffStrbufIgnCaseCompS(const FFstrbuf* strbuf, const char* comp) {
    return strcasecmp(strbuf->chars, comp);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufIgnCaseEqualS(const FFstrbuf* strbuf, const char* comp) {
    return ffStrbufIgnCaseCompS(strbuf, comp) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline int ffStrbufIgnCaseComp(const FFstrbuf* strbuf, const FFstrbuf* comp) {
    return ffStrbufIgnCaseCompS(strbuf, comp->chars);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufIgnCaseEqual(const FFstrbuf* strbuf, const FFstrbuf* comp) {
    return ffStrbufIgnCaseComp(strbuf, comp) == 0;
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline bool ffStrbufContainC(const FFstrbuf* strbuf, char c) {
    return memchr(strbuf->chars, c, strbuf->length) != nullptr;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufContainS(const FFstrbuf* strbuf, const char* str) {
    return strstr(strbuf->chars, str) != nullptr;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufContain(const FFstrbuf* strbuf, const FFstrbuf* str) {
    return ffStrbufContainS(strbuf, str->chars);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufContainIgnCaseS(const FFstrbuf* strbuf, const char* str) {
    return strcasestr(strbuf->chars, str) != nullptr;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufContainIgnCase(const FFstrbuf* strbuf, const FFstrbuf* str) {
    return ffStrbufContainIgnCaseS(strbuf, str->chars);
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline uint32_t ffStrbufNextIndexC(const FFstrbuf* strbuf, uint32_t start, char c) {
    assert(start <= strbuf->length);

    const char* ptr = (const char*) memchr(strbuf->chars + start, c, strbuf->length - start);
    return ptr ? (uint32_t) (ptr - strbuf->chars) : strbuf->length;
}

[[gnu::nonnull(1, 3), gnu::pure, nodiscard]] static inline uint32_t ffStrbufNextIndexS(const FFstrbuf* strbuf, uint32_t start, const char* str) {
    assert(start <= strbuf->length);

    const char* ptr = strstr(strbuf->chars + start, str);
    return ptr ? (uint32_t) (ptr - strbuf->chars) : strbuf->length;
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline uint32_t ffStrbufPreviousIndexC(const FFstrbuf* strbuf, uint32_t start, char c) {
    assert(start <= strbuf->length);

    const char* ptr = (const char*) memrchr(strbuf->chars, c, start + 1);
    return ptr ? (uint32_t) (ptr - strbuf->chars) : strbuf->length;
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline uint32_t ffStrbufFirstIndexC(const FFstrbuf* strbuf, char c) {
    return ffStrbufNextIndexC(strbuf, 0, c);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline uint32_t ffStrbufFirstIndex(const FFstrbuf* strbuf, const FFstrbuf* searched) {
    return ffStrbufNextIndexS(strbuf, 0, searched->chars);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline uint32_t ffStrbufFirstIndexS(const FFstrbuf* strbuf, const char* str) {
    return ffStrbufNextIndexS(strbuf, 0, str);
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline uint32_t ffStrbufLastIndexC(const FFstrbuf* strbuf, char c) {
    if (strbuf->length == 0) {
        return 0;
    }

    return ffStrbufPreviousIndexC(strbuf, strbuf->length - 1, c);
}

[[gnu::nonnull(1)]] static inline bool ffStrbufSubstrBeforeFirstC(FFstrbuf* strbuf, char c) {
    return ffStrbufSubstrBefore(strbuf, ffStrbufFirstIndexC(strbuf, c));
}

[[gnu::nonnull(1)]] static inline bool ffStrbufSubstrBeforeLastC(FFstrbuf* strbuf, char c) {
    return ffStrbufSubstrBefore(strbuf, ffStrbufLastIndexC(strbuf, c));
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWithC(const FFstrbuf* strbuf, char c) {
    return strbuf->chars[0] == c;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWithSN(const FFstrbuf* strbuf, const char* start, uint32_t length) {
    if (length > strbuf->length) {
        return false;
    }

    return memcmp(strbuf->chars, start, length) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWithS(const FFstrbuf* strbuf, const char* start) {
    return ffStrbufStartsWithSN(strbuf, start, (uint32_t) strlen(start));
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWith(const FFstrbuf* strbuf, const FFstrbuf* start) {
    return ffStrbufStartsWithSN(strbuf, start->chars, start->length);
}

[[gnu::nonnull(1, 3), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWithIgnCaseNS(const FFstrbuf* strbuf, uint32_t length, const char* start) {
    if (length > strbuf->length) {
        return false;
    }
    return strncasecmp(strbuf->chars, start, length) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWithIgnCaseS(const FFstrbuf* strbuf, const char* start) {
    return ffStrbufStartsWithIgnCaseNS(strbuf, (uint32_t) strlen(start), start);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufStartsWithIgnCase(const FFstrbuf* strbuf, const FFstrbuf* start) {
    return ffStrbufStartsWithIgnCaseNS(strbuf, start->length, start->chars);
}

[[gnu::nonnull(1), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWithC(const FFstrbuf* strbuf, char c) {
    return strbuf->length == 0 ? false : strbuf->chars[strbuf->length - 1] == c;
}

[[gnu::nonnull(1, 3), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWithNS(const FFstrbuf* strbuf, uint32_t endLength, const char* end) {
    if (endLength > strbuf->length) {
        return false;
    }

    return memcmp(strbuf->chars + strbuf->length - endLength, end, endLength) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWithS(const FFstrbuf* strbuf, const char* end) {
    return ffStrbufEndsWithNS(strbuf, (uint32_t) strlen(end), end);
}

// Not `pure`: the caller-supplied `fn` may have side effects
[[gnu::nonnull(1, 2), nodiscard]] static inline bool ffStrbufEndsWithFn(const FFstrbuf* strbuf, int (*const fn)(int)) {
    return strbuf->length == 0 ? false : fn(strbuf->chars[strbuf->length - 1]);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWith(const FFstrbuf* strbuf, const FFstrbuf* end) {
    return ffStrbufEndsWithNS(strbuf, end->length, end->chars);
}

[[gnu::nonnull(1, 3), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWithIgnCaseNS(const FFstrbuf* strbuf, uint32_t endLength, const char* end) {
    if (endLength > strbuf->length) {
        return false;
    }
    return strcasecmp(strbuf->chars + strbuf->length - endLength, end) == 0;
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWithIgnCaseS(const FFstrbuf* strbuf, const char* end) {
    return ffStrbufEndsWithIgnCaseNS(strbuf, (uint32_t) strlen(end), end);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufEndsWithIgnCase(const FFstrbuf* strbuf, const FFstrbuf* end) {
    return ffStrbufEndsWithIgnCaseNS(strbuf, end->length, end->chars);
}

[[gnu::nonnull(1)]] static inline void ffStrbufTrim(FFstrbuf* strbuf, char c) {
    ffStrbufTrimRight(strbuf, c);
    ffStrbufTrimLeft(strbuf, c);
}

[[gnu::nonnull(1)]] static inline void ffStrbufTrimSpace(FFstrbuf* strbuf) {
    ffStrbufTrimRightSpace(strbuf);
    ffStrbufTrimLeftSpace(strbuf);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufMatchSeparatedS(const FFstrbuf* strbuf, const char* comp, char separator) {
    return ffStrbufMatchSeparatedNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufMatchSeparated(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator) {
    return ffStrbufMatchSeparatedNS(strbuf, comp->length, comp->chars, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufMatchSeparatedIgnCaseS(const FFstrbuf* strbuf, const char* comp, char separator) {
    return ffStrbufMatchSeparatedIgnCaseNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufMatchSeparatedIgnCase(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator) {
    return ffStrbufMatchSeparatedIgnCaseNS(strbuf, comp->length, comp->chars, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufSeparatedContainS(const FFstrbuf* strbuf, const char* comp, char separator) {
    return ffStrbufSeparatedContainNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufSeparatedContain(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator) {
    return ffStrbufSeparatedContainNS(strbuf, comp->length, comp->chars, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufSeparatedContainIgnCaseS(const FFstrbuf* strbuf, const char* comp, char separator) {
    return ffStrbufSeparatedContainIgnCaseNS(strbuf, (uint32_t) strlen(comp), comp, separator);
}

[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] static inline bool ffStrbufSeparatedContainIgnCase(const FFstrbuf* strbuf, const FFstrbuf* comp, char separator) {
    return ffStrbufSeparatedContainIgnCaseNS(strbuf, comp->length, comp->chars, separator);
}

[[gnu::nonnull(1, 2)]] static inline void ffStrbufWriteTo(const FFstrbuf* strbuf, FILE* file) {
    fwrite(strbuf->chars, sizeof(*strbuf->chars), strbuf->length, file);
}

[[gnu::nonnull(1, 2)]] static inline void ffStrbufPutTo(const FFstrbuf* strbuf, FILE* file) {
    ffStrbufWriteTo(strbuf, file);
    fputc('\n', file);
}

// Not `pure`: `strtod` reads the LC_NUMERIC locale and writes errno
[[gnu::nonnull(1), nodiscard]] static inline double ffStrbufToDouble(const FFstrbuf* strbuf, double defaultValue) {
    char* str_end;
    double result = strtod(strbuf->chars, &str_end);
    return str_end == strbuf->chars ? defaultValue : result;
}

// Not `pure`: `strtoull` reads the LC_NUMERIC locale and writes errno
[[gnu::nonnull(1), nodiscard]] static inline uint64_t ffStrbufToUInt(const FFstrbuf* strbuf, uint64_t defaultValue) {
    char* str_end;
    unsigned long long result = strtoull(strbuf->chars, &str_end, 10);
    return str_end == strbuf->chars ? defaultValue : (uint64_t) result;
}

// Not `pure`: `strtoll` reads the LC_NUMERIC locale and writes errno
[[gnu::nonnull(1), nodiscard]] static inline int64_t ffStrbufToSInt(const FFstrbuf* strbuf, int64_t defaultValue) {
    char* str_end;
    long long result = strtoll(strbuf->chars, &str_end, 10);
    return str_end == strbuf->chars ? defaultValue : (int64_t) result;
}

// Returns true if the strbuf is modified
[[gnu::nonnull(1)]] [[gnu::nonnull(1)]] bool ffStrbufDecodeHexEscapeSequences(FFstrbuf* strbuf);

#define FF_STRBUF_AUTO_DESTROY [[gnu::cleanup(ffStrbufDestroy)]] FFstrbuf
#define FF_STRBUF_STATIC(str) { .allocated = 0, .length = (uint32_t) sizeof(str) - 1, .chars = str }
