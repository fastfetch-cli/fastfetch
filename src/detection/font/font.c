#include "font.h"
#include "detection/internal.h"

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* font);

static void detectFont(const FFinstance* instance, FFFontResult* font)
{
    ffStrbufInit(&font->error);

    for(uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
        ffStrbufInit(&font->fonts[i]);

    ffDetectFontImpl(instance, font);

    if(font->error.length > 0)
        return;

    for(uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
    {
        if(font->fonts[i].length > 0)
            return;
    }

    ffStrbufAppendS(&font->error, "No fonts found");
}

const FFFontResult* ffDetectFont(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFFontResult,
        detectFont(instance, &result);
    );
}
