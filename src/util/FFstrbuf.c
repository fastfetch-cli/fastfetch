#include "FFstrbuf.h"

#include <malloc.h>
#include <ctype.h>
#include <string.h>

void ffStrbufInit(FFstrbuf* strbuf)
{
    ffStrbufInitA(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC);
}

void ffStrbufInitCopy(FFstrbuf* strbuf, const FFstrbuf* src)
{
    ffStrbufInitA(strbuf, src->allocated);
    ffStrbufAppend(strbuf, src);
}

void ffStrbufInitS(FFstrbuf* strbuf, const char* value)
{
    ffStrbufInitAS(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC, value);
}

void ffStrbufInitNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    ffStrbufInitANS(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC, length, value);
}

void ffStrbufInitC(FFstrbuf* strbuf, const char c)
{
    ffStrbufInitAC(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC, c);
}

void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate)
{
    if(allocate == 0)
    {
        fprintf(stderr, "ffStrbufInitA(): allocation size == 0, changing to 1");
        allocate = 1;
    }

    strbuf->allocated = allocate;
    strbuf->chars = (char*) malloc(sizeof(char) * strbuf->allocated);

    ffStrbufClear(strbuf);
}

void ffStrbufInitAS(FFstrbuf* strbuf, uint32_t allocate, const char* value)
{
    ffStrbufInitA(strbuf, allocate);
    ffStrbufAppendS(strbuf, value);
}

void ffStrbufInitANS(FFstrbuf* strbuf, uint32_t allocate, uint32_t length, const char* value)
{
    ffStrbufInitA(strbuf, allocate);
    ffStrbufAppendNS(strbuf, length, value);
}

void ffStrbufInitAC(FFstrbuf* strbuf, uint32_t allocate, const char c)
{
    ffStrbufInitA(strbuf, allocate);
    ffStrbufAppendC(strbuf, c);
}

void ffStrbufInitF(FFstrbuf* strbuf, const char* format, ...)
{
    ffStrbufInitA(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    va_list arguments;
    va_start(arguments, format);
    ffStrbufAppendVF(strbuf, format, arguments);
    va_end(arguments);
}

void ffStrbufInitVF(FFstrbuf* strbuf, const char* format, va_list arguments)
{
    ffStrbufInitA(strbuf, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    ffStrbufAppendVF(strbuf, format, arguments);
}

void ffStrbufEnsureCapacity(FFstrbuf* strbuf, uint32_t allocate)
{
    if(strbuf->allocated >= allocate)
        return;
    strbuf->allocated = allocate;
    strbuf->chars = realloc(strbuf->chars, sizeof(char) * strbuf->allocated);
}

void ffStrbufEnsureFree(FFstrbuf* strbuf, uint32_t free)
{
    uint32_t allocate = strbuf->allocated;
    if(allocate < 2)
        allocate = 2;
    while((strbuf->length + free) > allocate)
        allocate *= 2;
    ffStrbufEnsureCapacity(strbuf, allocate);
}

void ffStrbufClear(FFstrbuf* strbuf)
{
    strbuf->chars[0] = '\0';
    strbuf->length = 0;
}

void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendNS(strbuf, value->length, value->chars);
}

void ffStrbufSetS(FFstrbuf* strbuf, const char* value)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendS(strbuf, value);
}

void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendNS(strbuf, length, value);
}

void ffStrbufSetC(FFstrbuf* strbuf, const char c)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendC(strbuf, c);
}

void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...)
{
    ffStrbufClear(strbuf);
    va_list arguments;
    va_start(arguments, format);
    ffStrbufAppendVF(strbuf, format, arguments);
    va_end(arguments);
}

void ffStrbufSetVF(FFstrbuf* strbuf, const char* format, va_list arguments)
{
    ffStrbufClear(strbuf);
    ffStrbufAppendVF(strbuf, format, arguments);
}

void ffStrbufAppend(FFstrbuf* strbuf, const FFstrbuf* value)
{
    if(value == NULL)
        return;
    ffStrbufAppendNS(strbuf, value->length, value->chars);
}

void ffStrbufAppendTransformS(FFstrbuf* strbuf, const char* value, int(*transformFunc)(int))
{
    if(value == NULL)
        return;

    for(uint32_t i = 0; value[i] != '\0'; i++)
    {
        if(i % 16 == 0)
            ffStrbufEnsureFree(strbuf, 16);
        strbuf->chars[strbuf->length++] = transformFunc(value[i]);
    }
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendS(FFstrbuf* strbuf, const char* value)
{
    if(value == NULL)
        return;

    for(uint32_t i = 0; value[i] != '\0'; i++)
    {
        if(i % 16 == 0)
            ffStrbufEnsureFree(strbuf, 16);
        strbuf->chars[strbuf->length++] = value[i];
    }
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value)
{
    if(value == NULL)
        return;

    ffStrbufEnsureFree(strbuf, length);

    for(uint32_t i = 0; i < length; i++)
    {
        if(value[i] == '\0')
            break;

        strbuf->chars[strbuf->length++] = value[i];
    }
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufAppendC(FFstrbuf* strbuf, const char c)
{
    ffStrbufEnsureFree(strbuf, 1);
    strbuf->chars[strbuf->length++] = c;
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

void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments)
{
    if(format == NULL)
        return;

    va_list localArguments;
    va_copy(localArguments, arguments);

    uint32_t written = (uint32_t) vsnprintf(strbuf->chars + strbuf->length, strbuf->allocated - strbuf->length, format, localArguments);

    va_end(localArguments);

    if(strbuf->length + written >= strbuf->allocated)
    {
        ffStrbufEnsureCapacity(strbuf, strbuf->allocated * 2);
        ffStrbufAppendVF(strbuf, format, arguments); //Try again with larger buffer
    }
    else
    {
        strbuf->length += written;
        strbuf->chars[strbuf->length] = '\0';
    }
}

char ffStrbufGetC(FFstrbuf* strbuf, uint32_t index)
{
    if(index >= strbuf->length)
        return '\0';

    return strbuf->chars[index];
}

void ffStrbufWriteTo(const FFstrbuf* strbuf, FILE* file)
{
    fwrite(strbuf->chars, sizeof(char), strbuf->length, file);
}

void ffStrbufPutTo(const FFstrbuf* strbuf, FILE* file)
{
    fwrite(strbuf->chars, sizeof(char), strbuf->length, file);
    fputc('\n', file);
}

int ffStrbufComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    return ffStrbufCompS(strbuf, comp->chars);
}

int ffStrbufCompS(const FFstrbuf* strbuf, const char* comp)
{
    return strcmp(strbuf->chars, comp);
}

int ffStrbufIgnCaseComp(const FFstrbuf* strbuf, const FFstrbuf* comp)
{
    return ffStrbufIgnCaseCompS(strbuf, comp->chars);
}

int ffStrbufIgnCaseCompS(const FFstrbuf* strbuf, const char* comp)
{
    return strcasecmp(strbuf->chars, comp);
}

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c)
{
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
    while(strbuf->length > 0 && strbuf->chars[strbuf->length - 1] == c)
        --strbuf->length;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufTrim(FFstrbuf* strbuf, char c)
{
    ffStrbufTrimRight(strbuf, c);
    ffStrbufTrimLeft(strbuf, c);
}

static bool strbufRemoveTest(FFstrbuf* strbuf, uint32_t i, const char* substr)
{
    uint32_t k;
    for(k = 0; substr[k] != '\0'; k++)
    {
        if(i + k > strbuf->length)
            return false;

        if(strbuf->chars[i + k] != substr[k])
            return false;
    }

    memmove(&strbuf->chars[i], &strbuf->chars[i + k], strbuf->length - i - k);
    strbuf->length -= k;
    strbuf->chars[strbuf->length] = '\0';
    return true;
}

void ffStrbufRemoveStringsA(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[])
{
    for(uint32_t k = 0; k < numStrings; k++)
    {
        for(uint32_t i = 0; i < strbuf->length; i++)
            while(strbufRemoveTest(strbuf, i, strings[k]));
    }
}

void ffStrbufRemoveStringsV(FFstrbuf* strbuf, uint32_t numStrings, va_list arguments)
{
    const char** strings = calloc(numStrings, sizeof(const char*));
    for(uint32_t i = 0; i < numStrings; i++)
        strings[i] = va_arg(arguments, const char*);

    ffStrbufRemoveStringsA(strbuf, numStrings, strings);

    free(strings);
}

void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, ...)
{
    va_list argp;
    va_start(argp, numStrings);
    ffStrbufRemoveStringsV(strbuf, numStrings, argp);
    va_end(argp);
}

uint32_t ffStrbufFirstIndexAfterC(const FFstrbuf* strbuf, uint32_t start, const char c)
{
    for(uint32_t i = start; i < strbuf->length; i++)
    {
        if(strbuf->chars[i] == c)
            return i;
    }
    return strbuf->length;
}

uint32_t ffStrbufFirstIndexC(const FFstrbuf* strbuf, const char c)
{
    return ffStrbufFirstIndexAfterC(strbuf, 0, c);
}

uint32_t ffStrbufFirstIndexAfterS(const FFstrbuf* strbuf, uint32_t start, const char* str)
{
    for(uint32_t i = start + 1; i < strbuf->length; i++)
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

uint32_t ffStrbufFirstIndexS(const FFstrbuf* strbuf, const char* str)
{
    return ffStrbufFirstIndexAfterS(strbuf, 0, str);
}

uint32_t ffStrbufLastIndexC(const FFstrbuf* strbuf, const char c)
{
    //We need to loop one higher than the actual index, because uint32_t is guranteed to be >= 0, so this statement would always be true
    for(uint32_t i = strbuf->length; i > 0; i--)
    {
        if(strbuf->chars[i - 1] == c)
            return i - 1;
    }
    return strbuf->length;
}

void ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index)
{
    if(strbuf->length <= index)
        return;

    strbuf->length = index;
    strbuf->chars[strbuf->length] = '\0';
}

void ffStrbufSubstrBeforeFirstC(FFstrbuf* strbuf, const char c)
{
    ffStrbufSubstrBefore(strbuf, ffStrbufFirstIndexC(strbuf, c));
}

void ffStrbufSubstrBeforeLastC(FFstrbuf* strbuf, const char c)
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

void ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, const char c)
{
    uint32_t index = ffStrbufFirstIndexC(strbuf, c);
    if(index < strbuf->length)
        ffStrbufSubstrAfter(strbuf, index);
}

void ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, const char c)
{
    uint32_t index = ffStrbufLastIndexC(strbuf, c);
    if(index < strbuf->length)
        ffStrbufSubstrAfter(strbuf, index);
}

bool ffStrbufStartsWith(const FFstrbuf* strbuf, const FFstrbuf* start)
{
    return ffStrbufStartsWithNS(strbuf, start->length, start->chars);
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

bool ffStrbufStartsWithNS(const FFstrbuf* strbuf, uint32_t length, const char* start)
{
    for(uint32_t i = 0; i < length; i++)
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
    return ffStrbufStartsWithNS(strbuf, start->length, start->chars);
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

bool ffStrbufStartsWithIgnCaseNS(const FFstrbuf* strbuf, uint32_t length, const char* start)
{
    for(uint32_t i = 0; i < length; i++)
    {
        if(i >= strbuf->length)
            return false;

        if(tolower(strbuf->chars[i]) != tolower(start[i]))
            return false;
    }
    return true;
}

void ffStrbufRecalculateLength(FFstrbuf* strbuf)
{
    for(strbuf->length = 0; strbuf->chars[strbuf->length] != '\0'; ++strbuf->length);
}

void ffStrbufDestroy(FFstrbuf* strbuf)
{
    free(strbuf->chars);
}
