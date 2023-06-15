#include "font.h"
#include "detection/internal.h"

const char* ffDetectFontImpl(const FFinstance* instance, FFFontResult* font);

const char* ffDetectFont(const FFinstance* instance, FFFontResult* font)
{
    const char* error = ffDetectFontImpl(instance, font);

    if(error) return error;

    for(uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
    {
        if(font->fonts[i].length > 0)
            return NULL;
    }

    return "No fonts found";
}
