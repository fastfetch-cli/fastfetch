#include "fastfetch.h"
#include "common/properties.h"
#include "common/printing.h"
#include "common/font.h"
#include "common/processing.h"
#include "detection/terminalfont/terminalfont.h"

#define FF_TERMFONT_MODULE_NAME "Terminal Font"
#define FF_TERMFONT_NUM_FORMAT_ARGS 4

void ffPrintTerminalFont(FFinstance* instance)
{
    FFfont font;
    const char* error = ffDetectTerminalFontCommon(instance, &font);
    if (error != NULL)
    {
        //Ensure font is not inited here
        return ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "%s", error);
    }

    if(font.pretty.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Terminal font is an empty value");
        ffFontDestroy(&font);
        return;
    }

    if(instance->config.terminalFont.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont.key);
        ffStrbufPutTo(&font.pretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, FF_TERMFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &font.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &font.styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font.pretty}
        });
    }
    ffFontDestroy(&font);
}
