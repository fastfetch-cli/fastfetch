#include "font.h"

const char* ffDetectFontImpl(FFFontResult* font);

const char* ffDetectFont(FFFontResult* font)
{
    const char* error = ffDetectFontImpl(font);

    if(error) return error;

    for(uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
    {
        if(font->fonts[i].length > 0)
            return NULL;
    }

    return "No fonts found";
}
