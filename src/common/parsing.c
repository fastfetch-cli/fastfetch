#include "fastfetch.h"

#include <string.h>

void ffGetGtkPretty(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4)
{
    if(gtk2->length > 0 && gtk3->length > 0 && gtk4->length > 0)
    {
        if((ffStrbufIgnCaseComp(gtk2, gtk3) == 0) && (ffStrbufIgnCaseComp(gtk2, gtk4) == 0))
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK2/3/4]");
        }
        else if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
        else if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0 && gtk3->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3]");
        }
    }
    else if(gtk3->length > 0 && gtk4->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0)
    {
        ffStrbufAppend(buffer, gtk2);
        ffStrbufAppendS(buffer, " [GTK2]");
    }
    else if(gtk3->length > 0)
    {
        ffStrbufAppend(buffer, gtk3);
        ffStrbufAppendS(buffer, " [GTK3]");
    }
    else if(gtk4->length > 0)
    {
        ffStrbufAppend(buffer, gtk4);
        ffStrbufAppendS(buffer, " [GTK4]");
    }
}

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

    uint32_t wordLength = *data - wordStart;
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

void ffFontInitCopy(FFfont* font, const char* name)
{
    fontInit(font);
    ffStrbufAppendS(&font->name, name);
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

static bool getPropValueLine(const char** line, const char* start, FFstrbuf* buffer)
{
    if(**line == '\0')
        return false;

    //Skip any amount of whitespace at the begin of line
    while(**line == ' ' || **line == '\t')
        ++(*line);

    while(*start != '\0')
    {
        // Any amount of whitespace in the format string matches any amount of whitespace in the line, even none
        if(*start == ' ' || *start == '\t')
        {
            while(*start == ' ' || *start == '\t')
                ++start;

            while(**line == ' ' || **line == '\t')
                ++(*line);

            continue;
        }

        //Line doesn't match start, skip it
        if(**line != *start || **line == '\0')
            return false;

        //Line and start match, continue testing
        ++(*line);
        ++start;
    }

    char valueEnd = '\n';

    //Allow faster parsing of XML
    if(*(*line - 1) == '>')
        valueEnd = '<';

    //Skip any amount of whitespace at the begin of the value
    while(**line == ' ' || **line == '\t')
        ++(*line);

    //Allow faster parsing of quotet values
    if(**line == '"' || **line == '\'')
    {
        valueEnd = **line;
        ++(*line);
    }

    //Copy the value to the buffer
    while(**line != valueEnd && **line != '\n' && **line != '\0')
    {
        ffStrbufAppendC(buffer, **line);
        ++(*line);
    }

    ffStrbufTrimRight(buffer, ' ');

    return true;
}

bool ffGetPropValue(const char* line, const char* start, FFstrbuf* buffer)
{
    return getPropValueLine(&line, start, buffer);
}

bool ffGetPropValueFromLines(const char* lines, const char* start, FFstrbuf* buffer)
{
    while(!getPropValueLine(&lines, start, buffer))
    {
        while(*lines != '\0' && *lines != '\n')
            ++lines;

        if(*lines == '\0')
            return false;

        //Skip '\n'
        ++lines;
    }

    return true;
}

void ffParseSemver(FFstrbuf* buffer, const FFstrbuf* major, const FFstrbuf* minor, const FFstrbuf* patch)
{
    if(major->length == 0 && minor->length == 0 && patch->length == 0)
        return;

    if(major->length > 0)
        ffStrbufAppend(buffer, major);
    else
        ffStrbufAppendC(buffer, '1');

    ffStrbufAppendC(buffer, '.');

    if(minor->length > 0)
        ffStrbufAppend(buffer, minor);
    else if(patch->length > 0)
        ffStrbufAppendC(buffer, '0');
    else
        return;

    ffStrbufAppendC(buffer, '.');

    ffStrbufAppend(buffer, patch);
}
