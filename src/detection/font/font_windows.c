#include "font.h"
#include "util/windows/unicode.h"

#include <windows.h>

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance);

    NONCLIENTMETRICSW info = { .cbSize = sizeof(info) };
    if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0))
        ffStrbufAppendS(&result->error, "SystemParametersInfoW(SPI_GETNONCLIENTMETRICS) failed");

    LOGFONTW* fonts[4] = { &info.lfCaptionFont, &info.lfMenuFont, &info.lfMessageFont, &info.lfStatusFont };

    for(uint32_t i = 0; i < sizeof(fonts) / sizeof(fonts[0]); ++i)
    {
        ffStrbufSetWS(&result->fonts[i], fonts[i]->lfFaceName);
        if(fonts[i]->lfHeight < 0)
            ffStrbufAppendF(&result->fonts[i], " (%dpt)", (int)-fonts[i]->lfHeight);
    }
}
