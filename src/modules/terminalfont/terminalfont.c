#include "fastfetch.h"
#include "common/printing.h"
#include "detection/terminalfont/terminalfont.h"
#include "modules/terminalfont/terminalfont.h"

#define FF_TERMINALFONT_DISPLAY_NAME "Terminal Font"
#define FF_TERMINALFONT_NUM_FORMAT_ARGS 4

void ffPrintTerminalFont(FFinstance* instance, FFTerminalFontOptions* options)
{
    const FFTerminalFontResult* terminalFont = ffDetectTerminalFont(instance);

    if(terminalFont->error.length > 0)
    {
        ffPrintError(instance, FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, "%s", terminalFont->error.chars);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs.key);
        ffStrbufPutTo(&terminalFont->font.pretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_TERMINALFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont->font.pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont->font.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont->font.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &terminalFont->font.styles}
        });
    }
}

void ffInitTerminalFontOptions(FFTerminalFontOptions* options)
{
    options->moduleName = FF_TERMINALFONT_MODULE_NAME;
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

#ifdef FF_HAVE_JSONC
void ffParseTerminalFontJsonObject(FFinstance* instance, json_object* module)
{
    FFTerminalFontOptions __attribute__((__cleanup__(ffDestroyTerminalFontOptions))) options;
    ffInitTerminalFontOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_TERMINALFONT_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTerminalFont(instance, &options);
}
#endif
