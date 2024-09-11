#include "fastfetch.h"
#include "common/format.h"
#include "common/parsing.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

#include <inttypes.h>

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg)
{
    switch(formatarg->type)
    {
        case FF_FORMAT_ARG_TYPE_INT:
            ffStrbufAppendF(buffer, "%" PRIi32, *(int32_t*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_UINT:
            ffStrbufAppendF(buffer, "%" PRIu32, *(uint32_t*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_UINT64:
            ffStrbufAppendF(buffer, "%" PRIu64, *(uint64_t*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_UINT16:
            ffStrbufAppendF(buffer, "%" PRIu16, *(uint16_t*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_UINT8:
            ffStrbufAppendF(buffer, "%" PRIu8, *(uint8_t*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_STRING:
            ffStrbufAppendS(buffer, (const char*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_STRBUF:
            ffStrbufAppend(buffer, (FFstrbuf*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_FLOAT:
            ffStrbufAppendF(buffer, "%f", *(float*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_DOUBLE:
            ffStrbufAppendF(buffer, "%g", *(double*)formatarg->value);
            break;
        case FF_FORMAT_ARG_TYPE_BOOL:
            ffStrbufAppendS(buffer, *(bool*)formatarg->value ? "true" : "false");
            break;
        case FF_FORMAT_ARG_TYPE_LIST:
        {
            const FFlist* list = formatarg->value;
            for(uint32_t i = 0; i < list->length; i++)
            {
                ffStrbufAppend(buffer, ffListGet(list, i));
                if(i < list->length - 1)
                    ffStrbufAppendS(buffer, ", ");
            }
            break;
        }
        default:
            if(formatarg->type != FF_FORMAT_ARG_TYPE_NULL)
                fprintf(stderr, "Error: format string \"%s\": argument is not implemented: %i\n", buffer->chars, formatarg->type);
            break;
    }
}

/**
 * @brief parses a string to a uint32_t
 *
 * If the string can't be parsed, or is < 1, uint32_t max is returned.
 *
 * @param placeholderValue the string to parse
 * @return uint32_t the parsed value
 */
static uint32_t getArgumentIndex(const char* placeholderValue, uint32_t numArgs, const FFformatarg* arguments)
{
    char firstChar = placeholderValue[0];
    if (firstChar == '\0')
        return 0; // use arg counter

    if (firstChar >= '0' && firstChar <= '9')
    {
        char* pEnd = NULL;
        uint32_t result = (uint32_t) strtoul(placeholderValue, &pEnd, 10);
        if (result > numArgs)
            return UINT32_MAX;
        if (*pEnd != '\0')
            return UINT32_MAX;
        return result;
    }
    else if (ffCharIsEnglishAlphabet(firstChar))
    {
        for (uint32_t i = 0; i < numArgs; ++i)
        {
            const FFformatarg* arg = &arguments[i];
            if (arg->name && strcasecmp(placeholderValue, arg->name) == 0)
                return i + 1;
        }
    }

    return UINT32_MAX;
}

static inline void appendInvalidPlaceholder(FFstrbuf* buffer, const char* start, const FFstrbuf* placeholderValue, uint32_t index, uint32_t formatStringLength)
{
    ffStrbufAppendS(buffer, start);
    ffStrbufAppend(buffer, placeholderValue);

    if(index < formatStringLength)
        ffStrbufAppendC(buffer, '}');
}

static inline bool formatArgSet(const FFformatarg* arg)
{
    return arg->value != NULL && (
        (arg->type == FF_FORMAT_ARG_TYPE_DOUBLE && *(double*)arg->value > 0.0) || //Also is false for NaN
        (arg->type == FF_FORMAT_ARG_TYPE_INT && *(int*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_STRBUF && ((FFstrbuf*)arg->value)->length > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_STRING && ffStrSet(arg->value)) ||
        (arg->type == FF_FORMAT_ARG_TYPE_UINT8 && *(uint8_t*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_UINT16 && *(uint16_t*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_UINT && *(uint32_t*)arg->value > 0) ||
        (arg->type == FF_FORMAT_ARG_TYPE_BOOL && *(bool*)arg->value) ||
        (arg->type == FF_FORMAT_ARG_TYPE_LIST && ((FFlist*)arg->value)->length > 0)
    );
}

void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, uint32_t numArgs, const FFformatarg* arguments)
{
    uint32_t argCounter = 0;

    uint32_t numOpenIfs = 0;
    uint32_t numOpenNotIfs = 0;

    FF_STRBUF_AUTO_DESTROY placeholderValue = ffStrbufCreate();

    for(uint32_t i = 0; i < formatstr->length; ++i)
    {
        // if we don't have a placeholder start just copy the chars over to output buffer
        if(formatstr->chars[i] != '{')
        {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
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

        ffStrbufClear(&placeholderValue);

        {
            uint32_t iEnd = ffStrbufNextIndexC(formatstr, i, '}');
            ffStrbufAppendNS(&placeholderValue, iEnd - i, &formatstr->chars[i]);
            i = iEnd;
        }

        char firstChar = placeholderValue.chars[0];

        if (placeholderValue.length == 1)
        {
            // test if for stop, if so break the loop
            if (firstChar == '-')
                break;

            // test for end of an if, if so do nothing
            if (firstChar == '?')
            {
                if(numOpenIfs == 0)
                    appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                else
                    --numOpenIfs;

                continue;
            }

            // test for end of a not if, if so do nothing
            if (firstChar == '/')
            {
                if(numOpenNotIfs == 0)
                    appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                else
                    --numOpenNotIfs;

                continue;
            }

            // test for end of a color, if so do nothing
            if (firstChar == '#')
            {
                if (!instance.config.display.pipe)
                    ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);

                continue;
            }
        }

        // test for if, if so evaluate it
        if (firstChar == '?')
        {
            ffStrbufSubstrAfter(&placeholderValue, 0);

            uint32_t index = getArgumentIndex(placeholderValue.chars, numArgs, arguments);

            // testing for an invalid index
            if (index > numArgs)
            {
                appendInvalidPlaceholder(buffer, "{?", &placeholderValue, i, formatstr->length);
                continue;
            }

            // continue normally if an format arg is set and the value is > 0
            if (formatArgSet(&arguments[index - 1]))
            {
                ++numOpenIfs;
                continue;
            }

            // fastforward to the end of the if without printing the in between
            i = ffStrbufNextIndexS(formatstr, i, "{?}") + 2; // 2 is the length of "{?}" - 1 because the loop will increment it again directly after continue
            continue;
        }

        // test for not if, if so evaluate it
        if (firstChar == '/')
        {
            ffStrbufSubstrAfter(&placeholderValue, 0);

            uint32_t index = getArgumentIndex(placeholderValue.chars, numArgs, arguments);

            // testing for an invalid index
            if (index > numArgs)
            {
                appendInvalidPlaceholder(buffer, "{/", &placeholderValue, i, formatstr->length);
                continue;
            }

            //continue normally if an format arg is not set or the value is 0
            if (!formatArgSet(&arguments[index - 1]))
            {
                ++numOpenNotIfs;
                continue;
            }

            // fastforward to the end of the if without printing the in between
            i = ffStrbufNextIndexS(formatstr, i, "{/}") + 2; // 2 is the length of "{/}" - 1 because the loop will increment it again directly after continue
            continue;
        }

        //test for color, if so evaluate it
        if (firstChar == '#')
        {
            if (!instance.config.display.pipe)
            {
                ffStrbufAppendS(buffer, "\e[");
                ffOptionParseColorNoClear(placeholderValue.chars + 1, buffer);
                ffStrbufAppendC(buffer, 'm');
            }
            continue;
        }

        //test for constant, if so evaluate it
        if (firstChar == '$')
        {
            char* pend = NULL;
            int32_t indexSigned = (int32_t) strtol(placeholderValue.chars + 1, &pend, 10);
            uint32_t index = (uint32_t) indexSigned;
            bool backward = indexSigned < 0;

            if (indexSigned == 0 || *pend != '\0' || instance.config.display.constants.length < index)
            {
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                continue;
            }

            FFstrbuf* item = FF_LIST_GET(FFstrbuf, instance.config.display.constants, backward
                ? instance.config.display.constants.length - index
                : index - 1);
            ffStrbufAppend(buffer, item);
            continue;
        }

        char* pSep = placeholderValue.chars;
        char cSep = '\0';
        while (*pSep && *pSep != ':' && *pSep != '<' && *pSep != '>' && *pSep != '~')
            ++pSep;
        if (*pSep)
        {
            cSep = *pSep;
            *pSep = '\0';
        }
        else
        {
            pSep = NULL;
        }

        uint32_t index = getArgumentIndex(placeholderValue.chars, numArgs, arguments);

        // test for invalid index
        if (index == 0)
            index = ++argCounter;

        if (index > numArgs)
        {
            if (pSep) *pSep = cSep;
            appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
            continue;
        }

        if (!cSep)
            ffFormatAppendFormatArg(buffer, &arguments[index - 1]);
        else if (cSep == '~')
        {
            FF_STRBUF_AUTO_DESTROY tempString = ffStrbufCreate();
            ffFormatAppendFormatArg(&tempString, &arguments[index - 1]);

            char* pEnd = NULL;
            int32_t start = (int32_t) strtol(pSep + 1, &pEnd, 10);
            if (start < 0)
                start = (int32_t) tempString.length + start;
            if (start >= 0 && (uint32_t) start < tempString.length)
            {
                if (*pEnd == '\0')
                    ffStrbufAppendNS(buffer, tempString.length - (uint32_t) start, &tempString.chars[start]);
                else if (*pEnd == ',')
                {
                    int32_t end = (int32_t) strtol(pEnd + 1, &pEnd, 10);
                    if (!*pEnd)
                    {
                        if (end < 0)
                            end = (int32_t) tempString.length + end;
                        if ((uint32_t) end > tempString.length)
                            end = (int32_t) tempString.length;
                        if (end > start)
                            ffStrbufAppendNS(buffer, (uint32_t) (end - start), &tempString.chars[start]);
                    }
                }
            }

            if (*pEnd)
            {
                *pSep = cSep;
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                continue;
            }
        }
        else
        {
            char* pEnd = NULL;
            int32_t truncLength = (int32_t) strtol(pSep + 1, &pEnd, 10);
            if (*pEnd != '\0')
            {
                *pSep = cSep;
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                continue;
            }

            bool ellipsis = false;
            if (truncLength < 0)
            {
                ellipsis = true;
                truncLength = -truncLength;
            }

            FF_STRBUF_AUTO_DESTROY tempString = ffStrbufCreate();
            ffFormatAppendFormatArg(&tempString, &arguments[index - 1]);
            if (tempString.length == (uint32_t) truncLength)
                ffStrbufAppend(buffer, &tempString);
            else if (tempString.length > (uint32_t) truncLength)
            {
                if (cSep == ':')
                {
                    ffStrbufSubstrBefore(&tempString, (uint32_t) truncLength);
                    ffStrbufTrimRightSpace(&tempString);
                }
                else
                    ffStrbufSubstrBefore(&tempString, (uint32_t) (!ellipsis? truncLength : truncLength - 1));
                ffStrbufAppend(buffer, &tempString);

                if (ellipsis)
                    ffStrbufAppendS(buffer, "…");
            }
            else if (cSep == ':')
                ffStrbufAppend(buffer, &tempString);
            else
            {
                if (cSep == '<')
                {
                    ffStrbufAppend(buffer, &tempString);
                    ffStrbufAppendNC(buffer, (uint32_t) truncLength - tempString.length, ' ');
                }
                else
                {
                    ffStrbufAppendNC(buffer, (uint32_t) truncLength - tempString.length, ' ');
                    ffStrbufAppend(buffer, &tempString);
                }
            }
        }
    }

    if (!instance.config.display.pipe)
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
}
