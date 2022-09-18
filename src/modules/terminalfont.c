#include "fastfetch.h"
#include "common/printing.h"
#include "detection/terminalfont/terminalfont.h"

#define FF_TERMFONT_MODULE_NAME "Terminal Font"
#define FF_TERMFONT_NUM_FORMAT_ARGS 4

void ffPrintTerminalFont(FFinstance* instance)
{
    const FFTerminalFontResult* terminalFont = ffDetectTerminalFont(instance);

    if(terminalFont->error.length > 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "%s", terminalFont->error.chars);
        return;
    }

    if(instance->config.terminalFont.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont.key);
        ffStrbufPutTo(&terminalFont->font.pretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, FF_TERMFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont->font.pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont->font.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont->font.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &terminalFont->font.styles}
        });
    }
}
