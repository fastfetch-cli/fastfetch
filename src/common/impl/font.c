#include "fastfetch.h"
#include "common/FFlist.h"
#include "common/FFstrbuf.h"
#include "common/stringUtils.h"
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
        if (!ffStrbufEndsWithS(&font->size, "pt") && !ffStrbufEndsWithS(&font->size, "px"))
            ffStrbufAppendS(&font->pretty, "pt");

        if(font->styles.length > 0)
            ffStrbufAppendS(&font->pretty, ", ");
    }

    for(uint32_t i = 0; i < font->styles.length; i++)
    {
        ffStrbufAppend(&font->pretty, FF_LIST_GET(FFstrbuf, font->styles, i));

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
            FFstrbuf* style = (FFstrbuf*) ffListAdd(&font->styles);
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
        ffStrStartsWithIgnCase(wordStart, "Ultra") ||
        ffStrStartsWithIgnCase(wordStart, "Extra") ||
        ffStrStartsWithIgnCase(wordStart, "Semi") ||
        ffStrStartsWithIgnCase(wordStart, "Demi") ||
        ffStrStartsWithIgnCase(wordStart, "Normal") ||
        ffStrStartsWithIgnCase(wordStart, "Roman") ||
        ffStrStartsWithIgnCase(wordStart, "Oblique") ||
        ffStrStartsWithIgnCase(wordStart, "Italic") ||
        ffStrStartsWithIgnCase(wordStart, "Thin") ||
        ffStrStartsWithIgnCase(wordStart, "Light") ||
        ffStrStartsWithIgnCase(wordStart, "Bold") ||
        ffStrStartsWithIgnCase(wordStart, "Black") ||
        ffStrStartsWithIgnCase(wordStart, "Condensed") ||
        ffStrStartsWithIgnCase(wordStart, "Expanded")
    ) {
        if(alternativeBuffer == NULL)
        {
            alternativeBuffer = (FFstrbuf*) ffListAdd(&font->styles);
            ffStrbufInit(alternativeBuffer);
        }

        strbufAppendNSExcludingC(alternativeBuffer, wordLength, wordStart, '-');

        if(
            ffStrStartsWithIgnCase(wordStart, "Ultra ") ||
            ffStrStartsWithIgnCase(wordStart, "Extra ") ||
            ffStrStartsWithIgnCase(wordStart, "Semi ") ||
            ffStrStartsWithIgnCase(wordStart, "Demi ")
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

void ffFontInitXlfd(FFfont* font, const char* xlfd)
{
    assert(xlfd && *xlfd);

    // https://en.wikipedia.org/wiki/X_logical_font_description
    ffFontInit(font);

    // XLFD: -foundry-family-weight-slant-setwidth-addstyle-pixelsize-pointsize-xres-yres-spacing-averagewidth-charsetregistry-charsetencoding
    // It often starts with '-', which would create an empty first field. Skip it to align indexes.
    if(*xlfd == '-')
        xlfd++;

    const char* pstart = xlfd;

    for(int field = 0; field < 14; field++)
    {
        const char* pend = strchr(pstart, '-');
        uint32_t length = pend ? (uint32_t)(pend - pstart) : (uint32_t) strlen(pstart);

        if(length > 0)
        {
            if(field == 1) // family
            {
                ffStrbufAppendNS(&font->name, length, pstart);
            }
            else if(field == 7) // pointsize (decipoints, preferred)
            {
                // parse positive integer from substring
                long deciPt = 0;
                bool ok = true;
                for(uint32_t i = 0; i < length; i++)
                {
                    char c = pstart[i];
                    if(c < '0' || c > '9') { ok = false; break; }
                    deciPt = deciPt * 10 + (c - '0');
                }

                if(ok && deciPt > 0)
                {
                    ffStrbufClear(&font->size);

                    char tmp[32];
                    if(deciPt % 10 == 0)
                        snprintf(tmp, sizeof(tmp), "%ldpt", deciPt / 10);
                    else
                        snprintf(tmp, sizeof(tmp), "%ld.%ldpt", deciPt / 10, deciPt % 10);

                    ffStrbufAppendS(&font->size, tmp);
                }
            }
            else if(field == 6) // pixelsize (fallback if pointsize missing/invalid)
            {
                if(font->size.length == 0)
                {
                    long px = 0;
                    bool ok = true;
                    for(uint32_t i = 0; i < length; i++)
                    {
                        char c = pstart[i];
                        if(c < '0' || c > '9') { ok = false; break; }
                        px = px * 10 + (c - '0');
                    }

                    if(ok && px > 0)
                    {
                        ffStrbufAppendNS(&font->size, length, pstart);
                        ffStrbufAppendS(&font->size, "px");
                    }
                }
            }
            else if(field >= 2 && field <= 5) // weight/slant/setwidth/addstyle
            {
                // ignore "normal" (case-insensitive)
                if(!(length == 6 && ffStrStartsWithIgnCase(pstart, "normal")))
                {
                    FFstrbuf* style = (FFstrbuf*) ffListAdd(&font->styles);
                    ffStrbufInitNS(style, length, pstart);
                }
            }
        }

        if(!pend)
            break;

        pstart = pend + 1;
    }

    fontInitPretty(font);
}

void ffFontInitXft(FFfont* font, const char* xft)
{
    assert(xft);

    // https://en.wikipedia.org/wiki/Xft
    // Xft/Fontconfig pattern examples:
    //   "DejaVu Sans Mono-10"
    //   "monospace:size=10:weight=bold:slant=italic"
    //   "Fira Code-12:style=Regular"
    // Goal: extract family(name), size, and some common styles.

    ffFontInit(font);

    // 1) Parse "head" part before first ':' => usually "family[-size]" (may include commas)
    const char* p = xft;

    while(*p == ' ' || *p == '\t')
        ++p;

    const char* headStart = p;
    while(*p != '\0' && *p != ':')
        ++p;
    const char* headEnd = p;

    // trim tail spaces
    while(headEnd > headStart && (headEnd[-1] == ' ' || headEnd[-1] == '\t'))
        --headEnd;

    // If multiple families are listed, take the first one (up to comma)
    for(const char* q = headStart; q < headEnd; ++q)
    {
        if(*q == ',')
        {
            headEnd = q;
            while(headEnd > headStart && (headEnd[-1] == ' ' || headEnd[-1] == '\t'))
                --headEnd;
            break;
        }
    }

    // Try parse trailing "-<number>" as size, otherwise entire head is name
    const char* dashPos = NULL;
    const char* sizeStart = NULL;

    for(const char* q = headEnd; q > headStart; )
    {
        --q;
        if(*q == '-' && (q + 1) < headEnd && ffCharIsDigit(q[1]))
        {
            dashPos = q;
            sizeStart = q + 1;
            break;
        }
    }

    if(dashPos)
    {
        bool ok = true;
        bool seenDigit = false;
        for(const char* q = sizeStart; q < headEnd; ++q)
        {
            if(ffCharIsDigit(*q))
                seenDigit = true;
            else if(*q == '.')
                continue;
            else
            {
                ok = false;
                break;
            }
        }

        if(ok && seenDigit)
        {
            const char* nameEnd = dashPos;
            while(nameEnd > headStart && (nameEnd[-1] == ' ' || nameEnd[-1] == '\t'))
                --nameEnd;

            if(nameEnd > headStart)
                ffStrbufAppendNS(&font->name, (uint32_t) (nameEnd - headStart), headStart);

            if(headEnd > sizeStart)
                ffStrbufAppendNS(&font->size, (uint32_t) (headEnd - sizeStart), sizeStart);
        }
        else
        {
            if(headEnd > headStart)
                ffStrbufAppendNS(&font->name, (uint32_t) (headEnd - headStart), headStart);
        }
    }
    else
    {
        if(headEnd > headStart)
            ffStrbufAppendNS(&font->name, (uint32_t) (headEnd - headStart), headStart);
    }

    ffStrbufTrim(&font->name, ' ');
    ffStrbufTrim(&font->name, '"');

    // 2) Parse key=value fields after ':' (Fontconfig-like). Fields separated by ':'.
    // Common keys: size, pixelsize, pointsize, style, weight, slant, width
    while(*p == ':')
    {
        ++p;

        // key
        const char* keyStart = p;
        while(*p != '\0' && *p != '=' && *p != ':')
            ++p;
        const char* keyEnd = p;

        if(*p != '=')
            continue; // skip tokens without '='

        ++p; // skip '='

        // value (until next ':', allow backslash-escaping)
        FF_STRBUF_AUTO_DESTROY value = ffStrbufCreate();

        while(*p != '\0' && *p != ':')
        {
            if(*p == '\\' && p[1] != '\0')
            {
                ++p;
                ffStrbufAppendC(&value, *p);
                ++p;
                continue;
            }

            ffStrbufAppendC(&value, *p);
            ++p;
        }

        ffStrbufTrim(&value, ' ');
        ffStrbufTrim(&value, '"');

        uint32_t keyLen = (uint32_t) (keyEnd - keyStart);

        // helper: set numeric size if not set yet
        const bool sizeEmpty = (font->size.length == 0);
        if(value.length > 0)
        {
            if(
                (keyLen == 4 && ffStrStartsWithIgnCase(keyStart, "size")) ||
                (keyLen == 9 && ffStrStartsWithIgnCase(keyStart, "pixelsize"))
            )
            {
                if(sizeEmpty && ffCharIsDigit(value.chars[0]))
                {
                    ffStrbufAppend(&font->size, &value);
                    ffStrbufAppendS(&font->size, keyLen == 4 ? "pt" : "px");
                }
            }
            else if(keyLen == 5 && ffStrStartsWithIgnCase(keyStart, "style"))
            {
                // style may contain multiple words: "Bold Italic"
                const char* s = value.chars;
                while(*s != '\0')
                {
                    while(*s == ' ' || *s == '\t' || *s == ',')
                        ++s;

                    const char* w = s;
                    while(*s != '\0' && *s != ' ' && *s != '\t' && *s != ',')
                        ++s;

                    if(s > w)
                    {
                        FFstrbuf* style = FF_LIST_ADD(FFstrbuf, font->styles);
                        ffStrbufInitNS(style, (uint32_t) (s - w), w);
                    }
                }
            }
            else if(
                (keyLen == 6 && ffStrStartsWithIgnCase(keyStart, "weight")) ||
                (keyLen == 5 && ffStrStartsWithIgnCase(keyStart, "slant")) ||
                (keyLen == 5 && ffStrStartsWithIgnCase(keyStart, "width"))
            ) {
                // normalize: remove '-' to align with other parsers ("Semi-Bold" -> "SemiBold")
                FFstrbuf* style = FF_LIST_ADD(FFstrbuf, font->styles);
                ffStrbufInit(style);
                strbufAppendNSExcludingC(style, value.length, value.chars, '-');
                ffStrbufTrim(style, ' ');
            }
        }
    }

    fontInitPretty(font);
}

void ffFontInitMoveValues(FFfont* font, FFstrbuf* name, FFstrbuf* size, FFstrbuf* style)
{
    ffFontInit(font);

    if (name) ffStrbufInitMove(&font->name, name);
    if (size) ffStrbufInitMove(&font->size, size);
    if (style)
    {
        FFstrbuf* styleBuf = FF_LIST_ADD(FFstrbuf, font->styles);
        ffStrbufInitMove(styleBuf, style);
    }

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

    FF_LIST_FOR_EACH(FFstrbuf, str, font->styles)
        ffStrbufDestroy(str);
    ffListDestroy(&font->styles);
}
