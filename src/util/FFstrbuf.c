#include "FFstrbuf.h"

#include <ctype.h>
#include <inttypes.h>
#include <math.h>

char* CHAR_NULL_PTR = "";

void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate)
{
    strbuf->allocated = allocate;

    if(strbuf->allocated > 0)
        strbuf->chars = (char*) malloc(sizeof(char) * strbuf->allocated);

    //This will set the length to zero and the null byte.
    ffStrbufClear(strbuf);
}

void ffStrbufInitVF(FFstrbuf* strbuf, const char* format, va_list arguments)
{
    assert(format != NULL);

    int len = vasprintf(&strbuf->chars, format, arguments);
    assert(len >= 0);

    strbuf->allocated = (uint32_t)(len + 1);
    strbuf->length = (uint32_t)len;
}

void ffStrbufEnsureFree(FFstrbuf* strbuf, uint32_t free)
{
    if(ffStrbufGetFree(strbuf) >= free && !(strbuf->allocated == 0 && strbuf->length > 0))
        return;

    uint32_t allocate = strbuf->allocated;
    if(allocate < FASTFETCH_STRBUF_DEFAULT_ALLOC)
        allocate = FASTFETCH_STRBUF_DEFAULT_ALLOC;

    while((strbuf->length + free + 1) > allocate) // + 1 for the null byte
        allocate *= 2;

    if(strbuf->allocated == 0)
    {
        char* newbuf = malloc(sizeof(*strbuf->chars) * allocate);
        if(strbuf->length == 0)
            *newbuf = '\0';
        else
            memcpy(newbuf, strbuf->chars, strbuf->length + 1);
        strbuf->chars = newbuf;
    }
    else
        strbuf->chars = realloc(strbuf->chars, sizeof(*strbuf->chars) * allocate);

    strbuf->allocated = allocate;
}

// for an empty buffer, free + 1 length memory will be allocated(+1 for the NUL)
void ffStrbufEnsureFixedLengthFree(FFstrbuf* strbuf, uint32_t free)
{
    uint32_t oldFree = ffStrbufGetFree(strbuf);
    if (oldFree >= free && !(strbuf->allocated == 0 && strbuf->length > 0))
        return;

    uint32_t newCap = strbuf->allocated + (free - oldFree);

    if(strbuf->allocated == 0)
    {
        newCap += strbuf->length + 1;
        char* newbuf = malloc(sizeof(*strbuf->chars) * newCap);
        if(strbuf->length == 0)
            *newbuf = '\0';
        else
            memcpy(newbuf, strbuf->chars, strbuf->length + 1);
        strbuf->chars = newbuf;
    }
    else
        strbuf->chars = realloc(strbuf->chars, sizeof(*strbuf->chars) * newCap);

    strbuf->allocated = newCap;
}

void ffStrbufClear(FFstrbuf* strbuf)
{
    assert(strbuf != NULL);

    if(strbuf->allocated == 0)
        strbuf->chars = CHAR_NULL_PTR;
    else
        strbuf->chars[0] = '\0';

    strbuf->length = 0;
}

void ffStrbufAppendC(FFstrbuf* strbuf, char c)
{
    ffStrbufEnsureFree(strbuf, 1);
    strbuf->chars[strbuf->length++] = c;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendNC(FFstrbuf* strbuf, uint32_t num, char c)
{
    if (num == 0) return;

    ffStrbufEnsureFree(strbuf, num);
    memset(&strbuf->chars[strbuf->length], c, num);
    strbuf->length += num;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    if(value == NULL || length == 0)
        return;

    ffStrbufEnsureFree(strbuf, length);
    memcpy(&strbuf->chars[strbuf->length], value, length);
    strbuf->length += length;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendTransformS(FFstrbuf* strbuf, const char* value, int(*transformFunc)(int))
{
    if(value == NULL)
        return;

    //Ensure capacity > 0 or the modification below will fail
    uint32_t length = (uint32_t) strlen(value);
    if(length == 0)
        return;

    ffStrbufEnsureFree(strbuf, length);
    for(uint32_t i = 0; value[i] != '\0'; i++)
    {
        strbuf->chars[strbuf->length++] = (char) transformFunc(value[i]);
    }
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments)
{
    assert(format != NULL);

    va_list copy;
    va_copy(copy, arguments);

    uint32_t free = ffStrbufGetFree(strbuf);
    int written = vsnprintf(strbuf->chars + strbuf->length, strbuf->allocated > 0 ? free + 1 : 0, format, arguments);

    if(written > 0 && strbuf->length + (uint32_t) written > free)
    {
        ffStrbufEnsureFree(strbuf, (uint32_t) written);
        written = vsnprintf(strbuf->chars + strbuf->length, (uint32_t) written + 1, format, copy);
    }

    va_end(copy);

    if(written > 0)
        strbuf->length += (uint32_t) written;
}

const char* ffStrbufAppendSUntilC(FFstrbuf* strbuf, const char* value, char until)
{
    if(value == NULL)
        return NULL;

    char* end = strchr(value, until);
    if(end == NULL)
        ffStrbufAppendS(strbuf, value);
    else
        ffStrbufAppendNS(strbuf, (uint32_t) (end - value), value);
    return end;
}

void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...)
{
    assert(format != NULL);

    va_list arguments;
    va_start(arguments, format);

    if(strbuf->allocated == 0) {
        ffStrbufInitVF(strbuf, format, arguments);
        va_end(arguments);
        return;
    }

    ffStrbufClear(strbuf);
    ffStrbufAppendVF(strbuf, format, arguments);
    va_end(arguments);
}

void ffStrbufAppendF(FFstrbuf* strbuf, const char* format, ...)
{
    assert(format != NULL);

    va_list arguments;
    va_start(arguments, format);
    ffStrbufAppendVF(strbuf, format, arguments);
    va_end(arguments);
}

void ffStrbufPrependNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    if(value == NULL || length == 0)
        return;

    ffStrbufEnsureFree(strbuf, length);
    memmove(strbuf->chars + length, strbuf->chars, strbuf->length + 1); // + 1 for the null byte
    memcpy(strbuf->chars, value, length);
    strbuf->length += length;
}

void ffStrbufPrependC(FFstrbuf* strbuf, char c)
{
    ffStrbufEnsureFree(strbuf, 1);
    memmove(strbuf->chars + 1, strbuf->chars, strbuf->length + 1); // + 1 for the null byte
    strbuf->chars[0] = c;
    strbuf->length += 1;
}

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c)
{
    if(strbuf->length == 0)
        return;

    uint32_t index = 0;
    while(index < strbuf->length && strbuf->chars[index] == c)
        ++index;

    if(index == 0)
        return;

    if(strbuf->allocated == 0)
    {
        //static string
        strbuf->length -= index;
        strbuf->chars += index;
        return;
    }

    memmove(strbuf->chars, strbuf->chars + index, strbuf->length - index);
    strbuf->length -= index;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufTrimRight(FFstrbuf* strbuf, char c)
{
    if (strbuf->length == 0)
        return;

    if (!ffStrbufEndsWithC(strbuf, c))
        return;

    do
        --strbuf->length;
    while (ffStrbufEndsWithC(strbuf, c));

    if (strbuf->allocated == 0)
    {
        //static string
        ffStrbufInitNS(strbuf, strbuf->length, strbuf->chars);
        return;
    }

    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufTrimLeftSpace(FFstrbuf* strbuf)
{
    if(strbuf->length == 0)
        return;

    uint32_t index = 0;
    while(index < strbuf->length && isspace(strbuf->chars[index]))
        ++index;

    if(index == 0)
        return;

    if(strbuf->allocated == 0)
    {
        //static string
        strbuf->length -= index;
        strbuf->chars += index;
        return;
    }

    memmove(strbuf->chars, strbuf->chars + index, strbuf->length - index);
    strbuf->length -= index;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufTrimRightSpace(FFstrbuf* strbuf)
{
    if (strbuf->length == 0)
        return;

    if (!ffStrbufEndsWithFn(strbuf, isspace))
        return;

    do
        --strbuf->length;
    while (ffStrbufEndsWithFn(strbuf, isspace));

    if (strbuf->allocated == 0)
    {
        //static string
        ffStrbufInitNS(strbuf, strbuf->length, strbuf->chars);
        return;
    }

    strbuf->chars[strbuf->length] = '\0';
}

bool ffStrbufRemoveSubstr(FFstrbuf* strbuf, uint32_t startIndex, uint32_t endIndex)
{
    if(startIndex > strbuf->length || startIndex >= endIndex)
        return false;

    if(endIndex > strbuf->length)
    {
        ffStrbufSubstrBefore(strbuf, startIndex);
        return true;
    }

    ffStrbufEnsureFree(strbuf, 0);
    memmove(strbuf->chars + startIndex, strbuf->chars + endIndex, strbuf->length - endIndex);
    strbuf->length -= (endIndex - startIndex);
    strbuf->chars[strbuf->length] = '\0';
    return true;
}

void ffStrbufRemoveS(FFstrbuf* strbuf, const char* str)
{
    uint32_t stringLength = (uint32_t) strlen(str);

    for(uint32_t i = ffStrbufNextIndexS(strbuf, 0, str); i < strbuf->length; i = ffStrbufNextIndexS(strbuf, i, str))
        ffStrbufRemoveSubstr(strbuf, i, i + stringLength);
}

void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[])
{
    for(uint32_t i = 0; i < numStrings; i++)
        ffStrbufRemoveS(strbuf, strings[i]);
}

uint32_t ffStrbufNextIndexC(const FFstrbuf* strbuf, uint32_t start, char c)
{
    assert(start <= strbuf->length);

    const char* ptr = (const char*)memchr(strbuf->chars + start, c, strbuf->length - start);
    return ptr ? (uint32_t)(ptr - strbuf->chars) : strbuf->length;
}

uint32_t ffStrbufNextIndexS(const FFstrbuf* strbuf, uint32_t start, const char* str)
{
    assert(start <= strbuf->length);

    const char* ptr = strstr(strbuf->chars + start, str);
    return ptr ? (uint32_t)(ptr - strbuf->chars) : strbuf->length;
}

uint32_t ffStrbufPreviousIndexC(const FFstrbuf* strbuf, uint32_t start, char c)
{
    assert(start <= strbuf->length);

    //We need to loop one higher than the actual index, because uint32_t is guaranteed to be >= 0, so this statement would always be true
    for(uint32_t i = start + 1; i > 0; i--)
    {
        if(strbuf->chars[i - 1] == c)
            return i - 1;
    }
    return strbuf->length;
}

void ffStrbufReplaceAllC(FFstrbuf* strbuf, char find, char replace)
{
    if (strbuf->length == 0)
        return;

    ffStrbufEnsureFree(strbuf, 0);
    for (char *current_pos = strchr(strbuf->chars, find); current_pos; current_pos = strchr(current_pos + 1, find))
        *current_pos = replace;
}

bool ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index)
{
    if(strbuf->length <= index)
        return false;

    if(strbuf->allocated == 0)
    {
        //static string
        if (index < strbuf->length)
            ffStrbufInitNS(strbuf, index, strbuf->chars);
        return true;
    }

    strbuf->length = index;
    strbuf->chars[strbuf->length] = '\0';
    return true;
}

bool ffStrbufSubstrAfter(FFstrbuf* strbuf, uint32_t index)
{
    if(index >= strbuf->length)
    {
        ffStrbufClear(strbuf);
        return true;
    }

    if(strbuf->allocated == 0)
    {
        //static string
        strbuf->length -= index + 1;
        strbuf->chars += index + 1;
        return true;
    }

    memmove(strbuf->chars, strbuf->chars + index + 1, strbuf->length - index - 1);
    strbuf->length -= (index + 1);
    strbuf->chars[strbuf->length] = '\0';
    return true;
}

bool ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, char c)
{
    uint32_t index = ffStrbufFirstIndexC(strbuf, c);
    if(index >= strbuf->length)
        return false;
    ffStrbufSubstrAfter(strbuf, index);
    return true;
}

bool ffStrbufSubstrAfterFirstS(FFstrbuf* strbuf, const char* str)
{
    if(*str == '\0')
        return false;

    uint32_t index = ffStrbufFirstIndexS(strbuf, str) + (uint32_t) strlen(str) - 1; // -1, because firstIndexS is already pointing to str[0], we want to add only the remaining length
    if(index >= strbuf->length)
        return false;

    ffStrbufSubstrAfter(strbuf, index);
    return true;
}

bool ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, char c)
{
    uint32_t index = ffStrbufLastIndexC(strbuf, c);
    if(index >= strbuf->length)
        return false;

    ffStrbufSubstrAfter(strbuf, index);
    return true;
}

bool ffStrbufSubstr(FFstrbuf* strbuf, uint32_t start, uint32_t end)
{
    if (__builtin_expect(start >= end, false))
    {
        ffStrbufClear(strbuf);
        return false;
    }

    if (__builtin_expect(start == 0, false)) return ffStrbufSubstrBefore(strbuf, end);
    if (__builtin_expect(end >= strbuf->length, false)) return ffStrbufSubstrAfter(strbuf, start - 1);

    uint32_t len = end - start;
    ffStrbufEnsureFixedLengthFree(strbuf, len); // In case of static string
    memmove(strbuf->chars, strbuf->chars + start, len);

    strbuf->length = len;
    strbuf->chars[len] = '\0';
    return true;
}

uint32_t ffStrbufCountC(const FFstrbuf* strbuf, char c)
{
    uint32_t result = 0;
    for(uint32_t i = 0; i < strbuf->length; i++)
    {
        if(strbuf->chars[i] == c)
            result++;
    }

    return result;
}


bool ffStrbufRemoveIgnCaseEndS(FFstrbuf* strbuf, const char* end)
{
    uint32_t endLength = (uint32_t) strlen(end);
    if(ffStrbufEndsWithIgnCaseNS(strbuf, endLength, end))
    {
        ffStrbufSubstrBefore(strbuf, strbuf->length - endLength);
        return true;
    }

    return false;
}

bool ffStrbufEnsureEndsWithC(FFstrbuf* strbuf, char c)
{
    if(ffStrbufEndsWithC(strbuf, c))
        return false;

    ffStrbufAppendC(strbuf, c);
    return true;
}

void ffStrbufWriteTo(const FFstrbuf* strbuf, FILE* file)
{
    fwrite(strbuf->chars, sizeof(*strbuf->chars), strbuf->length, file);
}

void ffStrbufPutTo(const FFstrbuf* strbuf, FILE* file)
{
    ffStrbufWriteTo(strbuf, file);
    fputc('\n', file);
}

double ffStrbufToDouble(const FFstrbuf* strbuf, double defaultValue)
{
    char* str_end;
    double result = strtod(strbuf->chars, &str_end);
    return str_end == strbuf->chars ? defaultValue : result;
}

uint64_t ffStrbufToUInt(const FFstrbuf* strbuf, uint64_t defaultValue)
{
    char* str_end;
    unsigned long long result = strtoull(strbuf->chars, &str_end, 10);
    return str_end == strbuf->chars ? defaultValue : (uint64_t)result;
}

int64_t ffStrbufToSInt(const FFstrbuf* strbuf, int64_t defaultValue)
{
    char* str_end;
    long long result = strtoll(strbuf->chars, &str_end, 10);
    return str_end == strbuf->chars ? defaultValue : (int64_t)result;
}

void ffStrbufAppendSInt(FFstrbuf* strbuf, int64_t value)
{
    ffStrbufEnsureFree(strbuf, 21); // Required by yyjson_write_number
    char* start = strbuf->chars + strbuf->length;

    yyjson_val val = {};
    unsafe_yyjson_set_sint(&val, value);
    char* end = yyjson_write_number(&val, start);

    assert(end != NULL);

    strbuf->length += (uint32_t)(end - start);
}

void ffStrbufAppendUInt(FFstrbuf* strbuf, uint64_t value)
{
    ffStrbufEnsureFree(strbuf, 21); // Required by yyjson_write_number
    char* start = strbuf->chars + strbuf->length;

    yyjson_val val = {};
    unsafe_yyjson_set_uint(&val, value);
    char* end = yyjson_write_number(&val, start);

    assert(end != NULL);

    strbuf->length += (uint32_t)(end - start);
}

void ffStrbufAppendDouble(FFstrbuf* strbuf, double value, int8_t precision, bool trailingZeros)
{
    assert(precision <= 15); // yyjson_write_number supports up to 15 digits after the decimal point

    ffStrbufEnsureFree(strbuf, 40); // Required by yyjson_write_number
    char* start = strbuf->chars + strbuf->length;

    if (precision == 0)
        value = round(value);
    yyjson_val val = {};
    unsafe_yyjson_set_double(&val, value);
    if (precision > 0)
        unsafe_yyjson_set_fp_to_fixed(&val, precision);

    // Write at most <precision> digits after the decimal point; doesn't append trailing zeros
    char* end = yyjson_write_number(&val, start);

    assert(end > start);

    strbuf->length += (uint32_t)(end - start);

    if (__builtin_expect(value > 1e21 || value < -1e21, false))
    {
        // If the value is too large, yyjson_write_number will write it in scientific notation
        return;
    }

    if (trailingZeros)
    {
        if (precision > 1)
        {
            for (char* p = end - 1; *p != '.' && p > start; --p)
                --precision;
            if (precision > 0)
                ffStrbufAppendNC(strbuf, (uint32_t) precision, '0');
        }
        else if (precision == 0 || (precision < 0 && end[-1] == '0'))
        {
            goto removeDecimalPoint;
        }
    }
    else
    {
        if (end[-1] == '0')
        {
        removeDecimalPoint:
            // yyjson always appends ".0" to make it a float point number. We need to remove it
            strbuf->length -= 2;
            strbuf->chars[strbuf->length] = '\0';
        }
    }
}

void ffStrbufUpperCase(FFstrbuf* strbuf)
{
    for (uint32_t i = 0; i < strbuf->length; ++i)
        strbuf->chars[i] = (char) toupper(strbuf->chars[i]);
}

void ffStrbufLowerCase(FFstrbuf* strbuf)
{
    for (uint32_t i = 0; i < strbuf->length; ++i)
        strbuf->chars[i] = (char) tolower(strbuf->chars[i]);
}

void ffStrbufInsertNC(FFstrbuf* strbuf, uint32_t index, uint32_t num, char c)
{
    if(num == 0) return;
    if (index >= strbuf->length)
        index = strbuf->length;

    ffStrbufEnsureFree(strbuf, num);
    memmove(strbuf->chars + index + num, strbuf->chars + index, strbuf->length - index + 1);
    memset(&strbuf->chars[index], c, num);
    strbuf->length += num;
}

/**
 * @brief Read a line from a FFstrbuf.
 *
 * @details Behaves like getline(3) but reads from a FFstrbuf.
 *
 * @param[in,out] lineptr The pointer to a pointer that will be set to the start of the line.
 *                         Can be NULL for the first call.
 * @param[in,out] n The pointer to the size of the buffer of lineptr.
 * @param[in] buffer The buffer to read from. The buffer must not be a string literal.
 *
 * @return true if a line has been read, false if the end of the buffer has been reached.
 */
bool ffStrbufGetline(char** lineptr, size_t* n, FFstrbuf* buffer)
{
    assert(lineptr && n && buffer);
    assert(buffer->allocated > 0 || (buffer->allocated == 0 && buffer->length == 0));
    assert(!*lineptr || (*lineptr >= buffer->chars && *lineptr <= buffer->chars + buffer->length));

    const char* pBufferEnd = buffer->chars + buffer->length;
    if (!*lineptr)
        *lineptr = buffer->chars;
    else
    {
        *lineptr += *n;
        if (*lineptr >= pBufferEnd) // non-empty last line
            return false;
        **lineptr = '\n';
        ++*lineptr;
    }
    if (*lineptr >= pBufferEnd) // empty last line
        return false;

    size_t remaining = (size_t) (pBufferEnd - *lineptr);
    char* ending = memchr(*lineptr, '\n', remaining);
    if (ending)
    {
        *n = (size_t) (ending - *lineptr);
        *ending = '\0';
    }
    else
        *n = remaining;
    return true;
}

/// @brief Restore the end of a line that was modified by ffStrbufGetline.
/// @warning This function should be called before breaking an ffStrbufGetline loop.
void ffStrbufGetlineRestore(char** lineptr, size_t* n, FFstrbuf* buffer)
{
    assert(buffer && lineptr && n);
    assert(buffer->allocated > 0 || (buffer->allocated == 0 && buffer->length == 0));
    assert(!*lineptr || (*lineptr >= buffer->chars && *lineptr <= buffer->chars + buffer->length));

    if (!*lineptr)
        return;

    *lineptr += *n;
    if (*lineptr < buffer->chars + buffer->length)
        **lineptr = '\n';
}

bool ffStrbufRemoveDupWhitespaces(FFstrbuf* strbuf)
{
    if (strbuf->allocated == 0) return false; // Doesn't work with static strings

    bool changed = false;
    for (uint32_t i = 0; i < strbuf->length; i++)
    {
        if (strbuf->chars[i] != ' ') continue;

        i++;
        uint32_t j = i;
        for (; j < strbuf->length && strbuf->chars[j] == ' '; j++);

        if (j == i) continue;
        memmove(&strbuf->chars[i], &strbuf->chars[j], strbuf->length - j + 1);
        strbuf->length -= j - i;
        changed = true;
    }

    return changed;
}

/// @brief Check if a separated string (comp) contains a substring (strbuf).
/// @param strbuf The substring to check.
/// @param compLength The length of the separated string to check.
/// @param comp The separated string to check.
/// @param separator The separator character.
bool ffStrbufMatchSeparatedNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator)
{
    if (strbuf->length == 0)
        return true;

    if (compLength == 0)
        return false;

    for (const char* p = comp; p < comp + compLength;)
    {
        const char* colon = memchr(p, separator, compLength);
        if (colon == NULL)
            return strcmp(strbuf->chars, p) == 0;

        uint32_t substrLength = (uint32_t) (colon - p);
        if (strbuf->length == substrLength && memcmp(strbuf->chars, p, substrLength) == 0)
            return true;

        p = colon + 1;
    }

    return false;
}

/// @brief Case insensitive version of ffStrbufMatchSeparatedNS.
bool ffStrbufMatchSeparatedIgnCaseNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator)
{
    if (strbuf->length == 0)
        return true;

    if (compLength == 0)
        return false;

    for (const char* p = comp; p < comp + compLength;)
    {
        const char* colon = memchr(p, separator, compLength);
        if (colon == NULL)
            return strcasecmp(strbuf->chars, p) == 0;

        uint32_t substrLength = (uint32_t) (colon - p);
        if (strbuf->length == substrLength && strncasecmp(strbuf->chars, p, substrLength) == 0)
            return true;

        p = colon + 1;
    }

    return false;
}

int ffStrbufAppendUtf32CodePoint(FFstrbuf* strbuf, uint32_t codepoint)
{
    if (codepoint <= 0x7F) {
        ffStrbufAppendC(strbuf, (char)codepoint);
        return 1;
    } else if (codepoint <= 0x7FF) {
        ffStrbufAppendNS(strbuf, 2, (char[]){
            (char) (0xC0 | (codepoint >> 6)),
            (char) (0x80 | (codepoint & 0x3F))
        });
        return 2;
    } else if (codepoint <= 0xFFFF) {
        ffStrbufAppendNS(strbuf, 3, (char[]){
            (char) (0xE0 | (codepoint >> 12)),
            (char) (0x80 | ((codepoint >> 6) & 0x3F)),
            (char) (0x80 | (codepoint & 0x3F))
        });
        return 3;
    } else if (codepoint <= 0x10FFFF) {
        ffStrbufAppendNS(strbuf, 4, (char[]){
            (char) (0xF0 | (codepoint >> 18)),
            (char) (0x80 | ((codepoint >> 12) & 0x3F)),
            (char) (0x80 | ((codepoint >> 6) & 0x3F)),
            (char) (0x80 | (codepoint & 0x3F))
        });
        return 4;
    }

    ffStrbufAppendS(strbuf, "�"); // U+FFFD REPLACEMENT CHARACTER
    return 1;
}

/// @brief Check if a separated string (strbuf) contains a substring (comp).
/// @param strbuf The separated to check.
/// @param compLength The length of the separated string to check.
/// @param comp The substring to check.
/// @param separator The separator character.
bool ffStrbufSeparatedContainNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator)
{
    uint32_t startIndex = 0;
    while(startIndex < strbuf->length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(strbuf, startIndex, separator);

        uint32_t folderLength = colonIndex - startIndex;
        if (folderLength == compLength && memcmp(strbuf->chars + startIndex, comp, compLength) == 0)
            return true;

        startIndex = colonIndex + 1;
    }

    return false;
}

bool ffStrbufSeparatedContainIgnCaseNS(const FFstrbuf* strbuf, uint32_t compLength, const char* comp, char separator)
{
    uint32_t startIndex = 0;
    while(startIndex < strbuf->length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(strbuf, startIndex, separator);

        uint32_t folderLength = colonIndex - startIndex;
        if (folderLength == compLength && strncasecmp(strbuf->chars + startIndex, comp, compLength) == 0)
            return true;

        startIndex = colonIndex + 1;
    }

    return false;
}
