#include "font.h"
#include "util/windows/unicode.h"

#include <windows.h>

static void generateString(FFFontResult* font)
{
    const char* types[] = { "Caption", "Menu", "Message", "Status" };
    for(uint32_t i = 0; i < ARRAY_SIZE(types); ++i)
    {
        if(i == 0 || !ffStrbufEqual(&font->fonts[i - 1], &font->fonts[i]))
        {
            if(i > 0)
                ffStrbufAppendS(&font->display, "], ");
            ffStrbufAppendF(&font->display, "%s [%s", font->fonts[i].chars, types[i]);
        }
        else
        {
            ffStrbufAppendS(&font->display, " / ");
            ffStrbufAppendS(&font->display, types[i]);
        }
    }
    ffStrbufAppendC(&font->display, ']');
}

const char* ffDetectFontImpl(FFFontResult* result)
{
    NONCLIENTMETRICSW info = { .cbSize = sizeof(info) };
    if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0))
        return "SystemParametersInfoW(SPI_GETNONCLIENTMETRICS) failed";

    LOGFONTW* fonts[4] = { &info.lfCaptionFont, &info.lfMenuFont, &info.lfMessageFont, &info.lfStatusFont };

    for(uint32_t i = 0; i < ARRAY_SIZE(fonts); ++i)
    {
        ffStrbufSetWS(&result->fonts[i], fonts[i]->lfFaceName);
        if(fonts[i]->lfHeight < 0)
            ffStrbufAppendF(&result->fonts[i], " (%dpt)", (int)-fonts[i]->lfHeight);
    }

    generateString(result);

    return NULL;
}
