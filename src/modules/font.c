#include "fastfetch.h"
#include "common/printing.h"
#include "detection/font/font.h"
#include "detection/displayserver/displayserver.h"

#define FF_FONT_MODULE_NAME "Font"
#define FF_FONT_NUM_FORMAT_ARGS (FF_DETECT_FONT_NUM_FONTS + 1)

void ffPrintFont(FFinstance* instance)
{
    const FFFontResult* font = ffDetectFont(instance);

    if(font->error.length > 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "%s", font->error.chars);
        return;
    }

    if(instance->config.font.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font.key);
        ffStrbufPutTo(&font->display, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, FF_FONT_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[0]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[1]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[2]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[3]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->display},
        });
    }
}
