#include "fastfetch.h"

#include <string.h>

static void fontInit(FFfont* font)
{
    ffStrbufInit(&font->pretty);
    ffStrbufInit(&font->name);
    ffStrbufInitA(&font->size, 4);
    ffListInitA(&font->styles, sizeof(FFstrbuf), 4);
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
    fontInit(font);

    //See https://doc.qt.io/qt-5/qfont.html#toString

    //Family
    while(*data != ',' && *data != '\0')
    {
        ffStrbufAppendC(&font->name, *data);
        ++data;
    }
    if(*data != '\0')
        ++data;
    ffStrbufTrim(&font->name, ' ');

    //Size
    while(*data != ',' && *data != '\0')
    {
        ffStrbufAppendC(&font->size, *data);
        ++data;
    }
    if(*data != '\0')
        ++data;
    ffStrbufTrim(&font->size, ' ');

    #define FF_FONT_QT_SKIP_VALUE \
        while(*data != ',' && *data != '\0') \
            ++data; \
        if(*data != '\0') \
             ++data;

    FF_FONT_QT_SKIP_VALUE //Pixel size
    FF_FONT_QT_SKIP_VALUE //Style hint
    FF_FONT_QT_SKIP_VALUE //Font weight
    FF_FONT_QT_SKIP_VALUE //Font style
    FF_FONT_QT_SKIP_VALUE //Underline
    FF_FONT_QT_SKIP_VALUE //Strike out
    FF_FONT_QT_SKIP_VALUE //Fixed pitch
    FF_FONT_QT_SKIP_VALUE //Always 0

    #undef FF_FONT_QT_SKIP_VALUE

    while(*data != '\0')
    {
        while(*data == ' ')
            ++data;

        if(*data == '\0')
            break;

        FFstrbuf* style = ffListAdd(&font->styles);
        ffStrbufInit(style);
        while(*data != ' ' && *data != '\0')
        {
            ffStrbufAppendC(style, *data);
            ++data;
        }
    }

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

        ffStrbufAppendNSExludingC(alternativeBuffer, wordLength, wordStart, '-');

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
        ffStrbufAppendNSExludingC(alternativeBuffer, wordLength, wordStart, '-');
        return;
    }

    if(font->name.length > 0)
        ffStrbufAppendC(&font->name, ' ');
    ffStrbufAppendNS(&font->name, wordLength, wordStart);
}

void ffFontInitPango(FFfont* font, const char* data)
{
    fontInit(font);

    while(*data != '\0' && *data != '`' && *data != '\\')
        fontPangoParseWord(&data, font, NULL);

    fontInitPretty(font);
}

void ffFontInitValues(FFfont* font, const char* name, const char* size)
{
    fontInit(font);

    ffStrbufAppendS(&font->name, name);
    ffStrbufAppendS(&font->size, size);

    fontInitPretty(font);
}

void ffFontInitCopy(FFfont* font, const char* name)
{
    ffFontInitValues(font, name, NULL);
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
