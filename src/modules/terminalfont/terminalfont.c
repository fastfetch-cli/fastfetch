#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalfont/terminalfont.h"
#include "modules/terminalfont/terminalfont.h"
#include "util/stringUtils.h"

#define FF_TERMINALFONT_DISPLAY_NAME "Terminal Font"
#define FF_TERMINALFONT_NUM_FORMAT_ARGS 4

void ffPrintTerminalFont(FFTerminalFontOptions* options)
{
    FFTerminalFontResult terminalFont;
    ffFontInit(&terminalFont.font);
    ffFontInit(&terminalFont.fallback);
    ffStrbufInit(&terminalFont.error);

    if(!ffDetectTerminalFont(&terminalFont))
    {
        ffPrintError(FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, "%s", terminalFont.error.chars);
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufWriteTo(&terminalFont.font.pretty, stdout);
            if(terminalFont.fallback.pretty.length)
            {
                fputs(" / ", stdout);
                ffStrbufWriteTo(&terminalFont.fallback.pretty, stdout);
            }
            putchar('\n');
        }
        else
        {
            ffPrintFormat(FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_TERMINALFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont.font.pretty},
                {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont.font.name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont.font.size},
                {FF_FORMAT_ARG_TYPE_LIST,   &terminalFont.font.styles}
            });
        }
    }

    ffStrbufDestroy(&terminalFont.error);
    ffFontDestroy(&terminalFont.font);
    ffFontDestroy(&terminalFont.fallback);
}

void ffInitTerminalFontOptions(FFTerminalFontOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_TERMINALFONT_MODULE_NAME, ffParseTerminalFontCommandOptions, ffParseTerminalFontJsonObject, ffPrintTerminalFont);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseTerminalFontCommandOptions(FFTerminalFontOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINALFONT_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseTerminalFontJsonObject(FFTerminalFontOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_TERMINALFONT_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
