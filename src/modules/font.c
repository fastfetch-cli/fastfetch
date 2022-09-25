#include "fastfetch.h"
#include "common/printing.h"
#include "detection/font/font.h"
#include "detection/displayserver/displayserver.h"

#define FF_FONT_MODULE_NAME "Font"
#define FF_FONT_NUM_FORMAT_ARGS 2

void ffPrintFont(FFinstance* instance)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "Font isn't supported in TTY");
        return;
    }

    FFlist list;
    ffListInit(&list, sizeof(FFFontResult));
    const char* error = ffDetectFontImpl(instance, &list);
    if(error)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "%s", error);
        return;
    }

    for(uint32_t i = 0; i < list.length; ++i)
    {
        FFFontResult* font = (FFFontResult*)ffListGet(&list, i);
        if(instance->config.font.outputFormat.length == 0)
        {
            FFstrbuf key;
            ffStrbufInitF(&key, "%s (%s)", FF_FONT_MODULE_NAME, font->type);
            ffPrintLogoAndKey(instance, key.chars, 0, &instance->config.font.key);
            puts(font->fontPretty.chars);
            ffStrbufDestroy(&key);
        }
        else
        {
            ffPrintFormat(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, FF_FONT_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRING, font->type},
                {FF_FORMAT_ARG_TYPE_STRBUF, &font->fontPretty}
            });
        }
        ffStrbufDestroy(&font->fontPretty);
    }

    ffListDestroy(&list);
}
