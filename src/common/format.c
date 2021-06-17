#include "fastfetch.h"

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg)
{
    if(formatarg->type == FF_FORMAT_ARG_TYPE_INT)
        ffStrbufAppendF(buffer, "%i", *(int*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_UINT)
        ffStrbufAppendF(buffer, "%u", *(uint32_t*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_UINT8)
        ffStrbufAppendF(buffer, "%hhu", *(uint8_t*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_STRING)
        ffStrbufAppendS(buffer, (const char*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_STRBUF)
        ffStrbufAppend(buffer, (FFstrbuf*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_DOUBLE)
        ffStrbufAppendF(buffer, "%g", *(double*)formatarg->value);
    else if(formatarg->type == FF_FORMAT_ARG_TYPE_LIST)
    {
        const FFlist* list = formatarg->value;
        for(uint32_t i = 0; i < list->length; i++)
        {
            ffStrbufAppend(buffer, ffListGet(list, i));
            if(i < list->length - 1)
                ffStrbufAppendS(buffer, ", ");
        }
    }
    else if(formatarg->type != FF_FORMAT_ARG_TYPE_NULL)
    {
        fprintf(stderr, "Error: format string \"%s\": argument is not implemented: %i\n", buffer->chars, formatarg->type);
    }
}

static inline bool placeholderValueIsForError(const FFstrbuf* placeholderValue)
{
    return
        ffStrbufIgnCaseCompS(placeholderValue, "e") == 0 ||
        ffStrbufIgnCaseCompS(placeholderValue, "error") == 0 ||
        ffStrbufIgnCaseCompS(placeholderValue, "0") == 0
    ;
}

static inline uint32_t getArgumentIndex(const FFstrbuf* placeholderValue)
{
    uint32_t result = UINT32_MAX;

    if(placeholderValue->chars[0] != '-')
        sscanf(placeholderValue->chars, "%u", &result);

    return result;
}

static inline void appendInvalidPlaceholder(FFstrbuf* buffer, const char* start, const FFstrbuf* placeholderValue, uint32_t index, uint32_t formatStringLength)
{
    ffStrbufAppendS(buffer, start);
    ffStrbufAppend(buffer, placeholderValue);

    if(index < formatStringLength)
        ffStrbufAppendC(buffer, '}');
}

static inline void appendEmptyPlaceholder(FFstrbuf* buffer, const char* placeholder, uint32_t* argCounter, uint32_t numArgs, const FFformatarg* arguments)
{
    if(*argCounter > numArgs)
        ffStrbufAppendS(buffer, placeholder);
    else
        ffFormatAppendFormatArg(buffer, &arguments[(*argCounter)++]);
}

static inline bool formatArgSet(const FFformatarg* arg)
{
    return arg->value != NULL && (
        (arg->type == FF_FORMAT_ARG_TYPE_DOUBLE && *(double*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_INT && *(int*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_STRBUF && ((FFstrbuf*)arg->value)->length > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_STRING && *(const char*)arg->value != '\0') ||
        (arg->type == FF_FORMAT_ARG_TYPE_UINT8 && *(uint8_t*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_UINT && *(uint32_t*)arg->value > 0)
    );
}

void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments)
{
    uint32_t argCounter = 0;

    uint32_t numOpenIfs = 0;
    uint32_t numOpenNotIfs = 0;
    uint32_t numOpenColors = 0;

    for(uint32_t i = 0; i < formatstr->length; ++i)
    {
        // if we don't have a placeholder start just copy the chars over to output buffer
        if(formatstr->chars[i] != '{')
        {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
            continue;
        }

        // if we have an { at the end handle it as {}
        if(i == formatstr->length - 1)
        {
            appendEmptyPlaceholder(buffer, "{", &argCounter, numArgs, arguments);
            continue;
        }

        // jump to next char, the start of the placeholder value
        ++i;

        // double {{ elvaluates to a single { and doesn't count as start
        if(formatstr->chars[i] == '{')
        {
            ffStrbufAppendC(buffer, '{');
            continue;
        }

        // placeholder is {}
        if(formatstr->chars[i] == '}')
        {
            appendEmptyPlaceholder(buffer, "{}", &argCounter, numArgs, arguments);
            continue;
        }

        FFstrbuf placeholderValue;
        ffStrbufInit(&placeholderValue);

        while(i < formatstr->length && formatstr->chars[i] != '}')
            ffStrbufAppendC(&placeholderValue, formatstr->chars[i++]);

        // test for error, if so print it
        if(placeholderValueIsForError(&placeholderValue)) {
            ffStrbufAppend(buffer, error);
            ffStrbufDestroy(&placeholderValue);
            continue;
        }

         // test if for stop, if so break the loop
        if(placeholderValue.length == 1 && placeholderValue.chars[0] == '-')
        {
            ffStrbufDestroy(&placeholderValue);
            break;
        }

        // test for end of an if, if so do nothing
        if(placeholderValue.length == 1 && placeholderValue.chars[0] == '?')
        {
            if(numOpenIfs == 0)
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
            else
                --numOpenIfs;

            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        // test for end of a not if, if so do nothing
        if(placeholderValue.length == 1 && placeholderValue.chars[0] == '/')
        {
            if(numOpenNotIfs == 0)
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
            else
                --numOpenNotIfs;

            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        // test for end of a color, if so do nothing
        if(placeholderValue.length == 1 && placeholderValue.chars[0] == '#')
        {
            if(numOpenColors == 0)
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
            else
            {
                ffStrbufAppendS(buffer, "\033[0m");
                --numOpenColors;
            }

            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        // test for if, if so evaluate it
        if(placeholderValue.chars[0] == '?')
        {
            ffStrbufSubstrAfter(&placeholderValue, 0);

            // continue if an error is set
            if(placeholderValueIsForError(&placeholderValue) && error != NULL && error->length > 0)
            {
                ++numOpenIfs;
                ffStrbufDestroy(&placeholderValue);
                continue;
            }

            uint32_t index = getArgumentIndex(&placeholderValue);

            // testing for an invalid index
            if(index > numArgs)
            {
                appendInvalidPlaceholder(buffer, "{?", &placeholderValue, i, formatstr->length);
                ffStrbufDestroy(&placeholderValue);
                continue;
            }

            // continue normally if an format arg is set and the value is > 0
            if(formatArgSet(&arguments[index - 1]))
            {
                ++numOpenIfs;
                ffStrbufDestroy(&placeholderValue);
                continue;
            }

            // fastforward to the end of the if without printing the in between
            i = ffStrbufFirstIndexAfterS(formatstr, i, "{?}") + 2; // 2 is the length of "{?}" -1 because the loop will increament it again directly after continue
            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        // test for not if, if so evaluate it
        if(placeholderValue.chars[0] == '/')
        {
            ffStrbufSubstrAfter(&placeholderValue, 0);

            //continue if an error os not set
            if(placeholderValueIsForError(&placeholderValue) && (error == NULL || error->length == 0))
            {
                ++numOpenNotIfs;
                ffStrbufDestroy(&placeholderValue);
                continue;
            }

            uint32_t index = getArgumentIndex(&placeholderValue);

            // testing for an invalid index
            if(index > numArgs)
            {
                appendInvalidPlaceholder(buffer, "{/", &placeholderValue, i, formatstr->length);
                ffStrbufDestroy(&placeholderValue);
                continue;
            }

            //continue normally if an format arg is not set or the value is 0
            if(!formatArgSet(&arguments[index - 1]))
            {
                ++numOpenNotIfs;
                ffStrbufDestroy(&placeholderValue);
                continue;
            }

            // fastforward to the end of the if without printing the in between
            i = ffStrbufFirstIndexAfterS(formatstr, i, "{/}") + 2; // 2 is the length of "{/}" -1 because the loop will increament it again directly after continue
            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        //test for color, if so evaluate it
        if(placeholderValue.chars[0] == '#')
        {
            ++numOpenColors;
            ffStrbufSubstrAfter(&placeholderValue, 0);
            ffStrbufAppendS(buffer, "\033[");
            ffStrbufAppend(buffer, &placeholderValue);
            ffStrbufAppendC(buffer, 'm');
            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        uint32_t index = getArgumentIndex(&placeholderValue);

        // test for invalid index
        if(index > numArgs)
        {
            appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
            ffStrbufDestroy(&placeholderValue);
            continue;
        }

        ffFormatAppendFormatArg(buffer, &arguments[index - 1]);

        ffStrbufDestroy(&placeholderValue);
    }

    ffStrbufTrimRight(buffer, ' ');

    if(numOpenColors > 0)
        ffStrbufAppendS(buffer, "\033[0m");
}
