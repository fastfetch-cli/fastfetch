#include "FFstrbuf.h"

#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

static char* CHAR_NULL_PTR = "";

void ffStrbufInit(FFstrbuf* strbuf)
{
    ffStrbufInitA(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC);
}

void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate)
{
    strbuf->allocated = allocate;

    if(strbuf->allocated > 0)
        strbuf->chars = (char*) malloc(sizeof(char) * strbuf->allocated);

    //This will set the length to zero and the null byte.
    ffStrbufClear(strbuf);
}

void ffStrbufInitCopy(FFstrbuf* strbuf, const FFstrbuf* src)
{
    ffStrbufInitA(strbuf, src->allocated);
    ffStrbufAppend(strbuf, src);
}

uint32_t ffStrbufGetFree(const FFstrbuf* strbuf)
{
    if(strbuf->allocated == 0)
        return 0;

    return strbuf->allocated - strbuf->length - 1; // - 1 for the null byte
}

void ffStrbufEnsureFree(FFstrbuf* strbuf, uint32_t free)
{
    if(ffStrbufGetFree(strbuf) >= free)
        return;

    uint32_t allocate = strbuf->allocated;
    if(allocate < 2)
        allocate = FASTFETCH_STRBUF_DEFAULT_ALLOC;

    while((strbuf->length + free + 1) > allocate) // + 1 for the null byte
        allocate *= 2;

    if(strbuf->allocated == 0)
    {
        strbuf->chars = malloc(sizeof(*strbuf->chars) * allocate);
        strbuf->chars[0] = '\0';
    }
    else
        strbuf->chars = realloc(strbuf->chars, sizeof(*strbuf->chars) * allocate);

    strbuf->allocated = allocate;
}

void ffStrbufClear(FFstrbuf* strbuf)
{
    if(strbuf->allocated == 0)
        strbuf->chars = CHAR_NULL_PTR;
    else
        strbuf->chars[0] = '\0';

    strbuf->length = 0;
}

void ffStrbufRecalculateLength(FFstrbuf* strbuf)
{
    for(strbuf->length = 0; strbuf->chars[strbuf->length] != '\0'; ++strbuf->length);
}

void ffStrbufAppend(FFstrbuf* strbuf, const FFstrbuf* value)
{
    if(value == NULL)
        return;
    ffStrbufAppendNS(strbuf, value->length, value->chars);
}

void ffStrbufAppendC(FFstrbuf* strbuf, char c)
{
    ffStrbufEnsureFree(strbuf, 1);
    strbuf->chars[strbuf->length++] = c;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    if(value == NULL)
        return;

    ffStrbufEnsureFree(strbuf, length);
    memcpy(&strbuf->chars[strbuf->length], value, length);
    strbuf->length += length;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendNSExludingC(FFstrbuf* strbuf, uint32_t length, const char* value, char exclude)
{
    if(value == NULL)
        return;

    ffStrbufEnsureFree(strbuf, length);

    for(uint32_t i = 0; i < length; i++)
    {
        if(value[i] != exclude)
        strbuf->chars[strbuf->length++] = value[i];
    }

    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendTransformS(FFstrbuf* strbuf, const char* value, int(*transformFunc)(int))
{
    if(value == NULL)
        return;

    for(uint32_t i = 0; value[i] != '\0'; i++)
    {
        if(i % 16 == 0)
            ffStrbufEnsureFree(strbuf, 16);
        strbuf->chars[strbuf->length++] = (char) transformFunc(value[i]);
    }
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments)
{
    if(format == NULL)
        return;

    va_list copy;
    va_copy(copy, arguments);

    uint32_t free = ffStrbufGetFree(strbuf);
    uint32_t written = (uint32_t) vsnprintf(strbuf->chars + strbuf->length, free, format, arguments);

    if(strbuf->length + written > free)
    {
        ffStrbufEnsureFree(strbuf, written);
        written = (uint32_t) vsnprintf(strbuf->chars + strbuf->length, ffStrbufGetFree(strbuf), format, copy);
    }

    va_end(copy);

    if(written == 0)
        return;

    strbuf->length += written;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendF(FFstrbuf* strbuf, const char* format, ...)
{
    if(format == NULL)
        return;

    va_list arguments;
    va_start(arguments, format);
    ffStrbufAppendVF(strbuf, format, arguments);
    va_end(arguments);
}

void ffStrbufPrependS(FFstrbuf* strbuf, const char* value)
{
    ffStrbufPrependNS(strbuf, (uint32_t) strlen(value), value);
}

void ffStrbufPrependNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    ffStrbufEnsureFree(strbuf, length);
    memmove(strbuf->chars + length, strbuf->chars, strbuf->length + 1); // + 1 for the null byte
    memcpy(strbuf->chars, value, length);
    strbuf->length += length;
}

void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendNS(strbuf, length, value);
}

void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendNS(strbuf, value->length, value->chars);
}

int ffStrbufComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    if(strbuf->length != comp->length)
        return (int) strbuf->length - (int) comp->length;

    return ffStrbufCompS(strbuf, comp->chars);
}

int ffStrbufCompS(const FFstrbuf* strbuf, const char* comp)
{
    return strcmp(strbuf->chars, comp);
}

int ffStrbufIgnCaseComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    if(strbuf->length != comp->length)
        return (int) strbuf->length - (int) comp->length;

    return ffStrbufIgnCaseCompS(strbuf, comp->chars);
}

int ffStrbufIgnCaseCompS(const FFstrbuf* strbuf, const char* comp)
{
    return strcasecmp(strbuf->chars, comp);
}

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c)
{
    if(strbuf->allocated == 0)
        return;

    uint32_t index = 0;
    while(index < strbuf->length && strbuf->chars[index] == c)
        ++index;

    if(index == 0)
        return;

    memmove(strbuf->chars, strbuf->chars + index, strbuf->length - index);
    strbuf->length -= index;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufTrimRight(FFstrbuf* strbuf, char c)
{
    if(strbuf->allocated == 0)
        return;

    while(ffStrbufEndsWithC(strbuf, c))
        --strbuf->length;

    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufTrim(FFstrbuf* strbuf, char c)
{
    ffStrbufTrimRight(strbuf, c);
    ffStrbufTrimLeft(strbuf, c);
}

void ffStrbufRemoveSubstr(FFstrbuf* strbuf, uint32_t startIndex, uint32_t endIndex)
{
    if(startIndex > strbuf->length || startIndex >= endIndex)
        return;

    if(endIndex > strbuf->length)
    {
        ffStrbufSubstrBefore(strbuf, startIndex);
        return;
    }

    memmove(strbuf->chars + startIndex, strbuf->chars + endIndex, strbuf->length - endIndex);
    strbuf->length -= (endIndex - startIndex);
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufRemoveS(FFstrbuf* strbuf, const char* str)
{
    uint32_t stringLength = (uint32_t) strlen(str);

    for(uint32_t i = ffStrbufNextIndexS(strbuf, 0, str); i < strbuf->length; i = ffStrbufNextIndexS(strbuf, i, str))
        ffStrbufRemoveSubstr(strbuf, i, i + stringLength);
}

void ffStrbufRemoveStringsA(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[])
{
    for(uint32_t i = 0; i < numStrings; i++)
        ffStrbufRemoveS(strbuf, strings[i]);
}

void ffStrbufRemoveStringsV(FFstrbuf* strbuf, uint32_t numStrings, va_list arguments)
{
    for(uint32_t i = 0; i < numStrings; i++)
        ffStrbufRemoveS(strbuf, va_arg(arguments, const char*));
}

void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, ...)
{
    va_list argp;
    va_start(argp, numStrings);
    ffStrbufRemoveStringsV(strbuf, numStrings, argp);
    va_end(argp);
}

uint32_t ffStrbufNextIndexC(const FFstrbuf* strbuf, uint32_t start, char c)
{
    for(uint32_t i = start; i < strbuf->length; i++)
    {
        if(strbuf->chars[i] == c)
            return i;
    }
    return strbuf->length;
}

uint32_t ffStrbufNextIndexS(const FFstrbuf* strbuf, uint32_t start, const char* str)
{
    for(uint32_t i = start; i < strbuf->length; i++)
    {
        bool found = true;

        for(uint32_t k = 0; str[k] != '\0'; k++)
        {
            if(i + k == strbuf->length)
                return strbuf->length;

            if(strbuf->chars[i + k] != str[k])
            {
                found = false;
                break;
            }
        }

        if(found)
            return i;
    }

    return strbuf->length;
}

uint32_t ffStrbufFirstIndexC(const FFstrbuf* strbuf, char c)
{
    return ffStrbufNextIndexC(strbuf, 0, c);
}

uint32_t ffStrbufFirstIndex(const FFstrbuf* strbuf, const FFstrbuf* searched)
{
    return ffStrbufNextIndexS(strbuf, 0, searched->chars);
}

uint32_t ffStrbufFirstIndexS(const FFstrbuf* strbuf, const char* str)
{
    return ffStrbufNextIndexS(strbuf, 0, str);
}

uint32_t ffStrbufPreviousIndexC(const FFstrbuf* strbuf, uint32_t start, char c)
{
    if(start >= strbuf->length)
        return strbuf->length;

    //We need to loop one higher than the actual index, because uint32_t is guranteed to be >= 0, so this statement would always be true
    for(uint32_t i = start + 1; i > 0; i--)
    {
        if(strbuf->chars[i - 1] == c)
            return i - 1;
    }
    return strbuf->length;
}

uint32_t ffStrbufLastIndexC(const FFstrbuf* strbuf, char c)
{
    return ffStrbufPreviousIndexC(strbuf, strbuf->length - 1, c);
}

void ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index)
{
    if(strbuf->length <= index)
        return;

    strbuf->length = index;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufSubstrBeforeFirstC(FFstrbuf* strbuf, char c)
{
    ffStrbufSubstrBefore(strbuf, ffStrbufFirstIndexC(strbuf, c));
}

void ffStrbufSubstrBeforeLastC(FFstrbuf* strbuf, char c)
{
    ffStrbufSubstrBefore(strbuf, ffStrbufLastIndexC(strbuf, c));
}

void ffStrbufSubstrAfter(FFstrbuf* strbuf, uint32_t index)
{
    if(index >= strbuf->length)
    {
        ffStrbufClear(strbuf);
        return;
    }

    memmove(strbuf->chars, strbuf->chars + index + 1, strbuf->length - index - 1);
    strbuf->length -= (index + 1);
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, char c)
{
    uint32_t index = ffStrbufFirstIndexC(strbuf, c);
    if(index < strbuf->length)
        ffStrbufSubstrAfter(strbuf, index);
}

void ffStrbufSubstrAfterFirstS(FFstrbuf* strbuf, const char* str)
{
    if(*str == '\0')
        return;

    uint32_t index = ffStrbufFirstIndexS(strbuf, str) + (uint32_t) strlen(str) - 1; // -1, because firstIndexS is already pointing to str[0], we want to add only the remaining length
    if(index < strbuf->length)
        ffStrbufSubstrAfter(strbuf, index);
}

void ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, char c)
{
    uint32_t index = ffStrbufLastIndexC(strbuf, c);
    if(index < strbuf->length)
        ffStrbufSubstrAfter(strbuf, index);
}

bool ffStrbufStartsWithS(const FFstrbuf* strbuf, const char* start)
{
    for(uint32_t i = 0; start[i] != '\0'; i++)
    {
        if(i >= strbuf->length)
            return false;

        if(strbuf->chars[i] != start[i])
            return false;
    }
    return true;
}

bool ffStrbufStartsWithIgnCase(const FFstrbuf* strbuf, const FFstrbuf* start)
{
    return ffStrbufStartsWithS(strbuf, start->chars);
}

bool ffStrbufStartsWithIgnCaseS(const FFstrbuf* strbuf, const char* start)
{
    for(uint32_t i = 0; start[i] != '\0'; i++)
    {
        if(i >= strbuf->length)
            return false;

        if(tolower(strbuf->chars[i]) != tolower(start[i]))
            return false;
    }
    return true;
}

bool ffStrbufEndsWithC(const FFstrbuf* strbuf, char c)
{
    return strbuf->length == 0 ? false :
        strbuf->chars[strbuf->length - 1] == c;
}

bool ffStrbufEndsWithS(const FFstrbuf* strbuf, const char* end)
{
    uint32_t endLength = (uint32_t) strlen(end);

    if(endLength > strbuf->length)
        return false;

    for(uint32_t i = endLength; i > 0; i--)
    {
        if(strbuf->chars[strbuf->length - endLength + i - 1] != end[i - 1])
            return false;
    }

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

static bool testEndsWithIgnCaseS(const FFstrbuf* strbuf, const char* end, uint32_t* endLength)
{
    *endLength = (uint32_t) strlen(end);

    if(*endLength > strbuf->length)
        return false;

    for(uint32_t i = 0; i < *endLength; ++i)
    {
        if(tolower(strbuf->chars[strbuf->length - *endLength + i]) != tolower(end[i]))
            return false;
    }

    return true;
}

bool ffStrbufRemoveIgnCaseEndS(FFstrbuf* strbuf, const char* end)
{
    uint32_t endLength;
    if(testEndsWithIgnCaseS(strbuf, end, &endLength))
    {
        ffStrbufSubstrBefore(strbuf, strbuf->length - endLength);
        return true;
    }

    return false;
}

void ffStrbufEnsureEndsWithC(FFstrbuf* strbuf, char c)
{
    if(!ffStrbufEndsWithC(strbuf, c))
        ffStrbufAppendC(strbuf, c);
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

double ffStrbufToDouble(const FFstrbuf* strbuf)
{
    double value;
    if(sscanf(strbuf->chars, "%lf", &value) != 1)
        return 0.0 / 0.0; //NaN

    return value;
}

uint16_t ffStrbufToUInt16(const FFstrbuf* strbuf, uint16_t defaultValue)
{
    uint16_t value;
    if(sscanf(strbuf->chars, "%"SCNu16, &value) != 1)
        return defaultValue;

    return value;
}

void ffStrbufDestroy(FFstrbuf* strbuf)
{
    if(strbuf->allocated == 0) return;

    //Avoid free-after-use. These 3 assignments are cheap so don't remove them
    strbuf->allocated = strbuf->length = 0;
    free(strbuf->chars);
    strbuf->chars = NULL;
}

#ifdef __APPLE__

void ffStrbufAppendS(FFstrbuf* strbuf, const char* value)
{
    if(value == NULL)
        return;
    ffStrbufAppendNS(strbuf, (uint32_t) strlen(value), value);
}

void ffStrbufSetS(FFstrbuf* strbuf, const char* value)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendNS(strbuf, (uint32_t) strlen(value), value);
}

void ffStrbufInitS(FFstrbuf* strbuf, const char* str)
{
    ffStrbufInitA(strbuf, 0);
    ffStrbufAppendS(strbuf, str);
}

#endif
