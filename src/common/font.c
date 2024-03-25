#include "fastfetch.h"
#include "common/font.h"

#include <string.h>
#include <ctype.h>

void ffFontInit(FFfont* font)
{
    // Ensure no memory allocates
    ffStrbufInit(&font->pretty);
    ffStrbufInit(&font->name);
    ffStrbufInit(&font->size);
    ffListInit(&font->styles, sizeof(FFstrbuf));
}

static void strbufAppendNSExcludingC(FFstrbuf* strbuf, uint32_t length, const char* value, char exclude)
{
    if(value == NULL || length == 0)
        return;

    ffStrbufEnsureFree(strbuf, length);

    for(uint32_t i = 0; i < length; i++)
    {
        if(value[i] != exclude)
        strbuf->chars[strbuf->length++] = value[i];
    }

    strbuf->chars[strbuf->length] = '\0';
}

static void fontInitPretty(FFfont* font)
{
    ffStrbufAppend(&font->pretty, &font->name);

    if(font->size.length == 0 && font->styles.length == 0)
        return;
    else if(font->pretty.length == 0)
        ffStrbufAppendS(&font->pretty, "default");

    ffStrbufAppendS(&font->pretty, " (");

    if(font->size.length > 0)
    {
        ffStrbufAppend(&font->pretty, &font->size);
        ffStrbufAppendS(&font->pretty, "pt");

        if(font->styles.length > 0)
            ffStrbufAppendS(&font->pretty, ", ");
    }

    for(uint32_t i = 0; i < font->styles.length; i++)
    {
        ffStrbufAppend(&font->pretty, ffListGet(&font->styles, i));

        if(i < font->styles.length - 1)
            ffStrbufAppendS(&font->pretty, ", ");
    }

    ffStrbufAppendC(&font->pretty, ')');
}

void ffFontInitQt(FFfont* font, const char* data)
{
    ffFontInit(font);

    //See https://doc.qt.io/qt-5/qfont.html#toString

    //Family
    data = ffStrbufAppendSUntilC(&font->name, data, ',');
    ffStrbufTrim(&font->name, ' ');
    if (!data) goto exit;
    data++;

    //Size
    data = ffStrbufAppendSUntilC(&font->size, data, ',');
    ffStrbufTrim(&font->size, ' ');
    if (!data) goto exit;
    data++;

    //Style
    data = strrchr(data, ',');
    if (!data) goto exit;
    data++;
    if (isalpha(*data))
    {
        do
        {
            FFstrbuf* style = ffListAdd(&font->styles);
            ffStrbufInit(style);
            data = ffStrbufAppendSUntilC(style, data, ' ');
            if (data) data++;
        } while (data);
    }

exit:
    fontInitPretty(font);
}

static void fontPangoParseWord(const char** data, FFfont* font, FFstrbuf* alternativeBuffer)
{
    while(**data == ' ' || **data == '\t' || **data == ',')
        ++(*data);

    const char* wordStart = *data;

    while(**data != ' ' && **data != '\t' && **data != ',' && **data != '\0' && **data != '`' && **data != '\\')
        ++(*data);

    uint32_t wordLength = (uint32_t) (*data - wordStart);
    if(wordLength == 0)
        return;

    if(**data == '\0' || **data == '`' || **data == '\\')
    {
        ffStrbufAppendNS(&font->size, wordLength, wordStart);
        if(ffStrbufEndsWithS(&font->size, "px"))
            ffStrbufSubstrBefore(&font->size, font->size.length - 2);

        double dummy;
        if(sscanf(font->size.chars, "%lf", &dummy) == 1)
            return;

        ffStrbufClear(&font->size);
    }

    if(
        strncasecmp(wordStart, "Ultra", 5) == 0 ||
        strncasecmp(wordStart, "Extra", 5) == 0 ||
        strncasecmp(wordStart, "Semi", 4) == 0 ||
        strncasecmp(wordStart, "Demi", 4) == 0 ||
        strncasecmp(wordStart, "Normal", wordLength) == 0 ||
        strncasecmp(wordStart, "Roman", wordLength) == 0 ||
        strncasecmp(wordStart, "Oblique", wordLength) == 0 ||
        strncasecmp(wordStart, "Italic", wordLength) == 0 ||
        strncasecmp(wordStart, "Thin", wordLength) == 0 ||
        strncasecmp(wordStart, "Light", wordLength) == 0 ||
        strncasecmp(wordStart, "Bold", wordLength) == 0 ||
        strncasecmp(wordStart, "Black", wordLength) == 0 ||
        strncasecmp(wordStart, "Condensed", wordLength) == 0 ||
        strncasecmp(wordStart, "Expanded", wordLength) == 0
    ) {
        if(alternativeBuffer == NULL)
        {
            alternativeBuffer = ffListAdd(&font->styles);
            ffStrbufInit(alternativeBuffer);
        }

        strbufAppendNSExcludingC(alternativeBuffer, wordLength, wordStart, '-');

        if(
            strncasecmp(wordStart, "Ultra ", 6) == 0 ||
            strncasecmp(wordStart, "Extra ", 6) == 0 ||
            strncasecmp(wordStart, "Semi ", 5) == 0 ||
            strncasecmp(wordStart, "Demi ", 5) == 0
        ) {
            fontPangoParseWord(data, font, alternativeBuffer);
        }

        return;
    }

    if(alternativeBuffer != NULL)
    {
        strbufAppendNSExcludingC(alternativeBuffer, wordLength, wordStart, '-');
        return;
    }

    if(font->name.length > 0)
        ffStrbufAppendC(&font->name, ' ');
    ffStrbufAppendNS(&font->name, wordLength, wordStart);
}

void ffFontInitPango(FFfont* font, const char* data)
{
    ffFontInit(font);

    while(*data != '\0' && *data != '`' && *data != '\\')
        fontPangoParseWord(&data, font, NULL);

    fontInitPretty(font);
}

void ffFontInitValues(FFfont* font, const char* name, const char* size)
{
    ffFontInit(font);

    ffStrbufAppendS(&font->name, name);
    ffStrbufTrim(&font->name, '"');
    ffStrbufAppendS(&font->size, size);

    fontInitPretty(font);
}

void ffFontInitWithSpace(FFfont* font, const char* rawName)
{
    const char* pspace = strrchr(rawName, ' ');
    if(pspace == NULL)
    {
        ffFontInitCopy(font, rawName);
        return;
    }

    ffFontInit(font);

    ffStrbufAppendNS(&font->name, (uint32_t)(pspace - rawName), rawName);
    ffStrbufAppendS(&font->size, pspace + 1);

    fontInitPretty(font);
}

void ffFontDestroy(FFfont* font)
{
    ffStrbufDestroy(&font->pretty);
    ffStrbufDestroy(&font->name);
    ffStrbufDestroy(&font->size);

    for(uint32_t i = 0; i < font->styles.length; i++)
        ffStrbufDestroy(ffListGet(&font->styles, i));
    ffListDestroy(&font->styles);
}
