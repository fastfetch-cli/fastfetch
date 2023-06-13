#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalfont/terminalfont.h"
#include "modules/terminalfont/terminalfont.h"

#define FF_TERMINALFONT_DISPLAY_NAME "Terminal Font"
#define FF_TERMINALFONT_NUM_FORMAT_ARGS 4

void ffPrintTerminalFont(FFinstance* instance, FFTerminalFontOptions* options)
{
    FFTerminalFontResult terminalFont;
    ffFontInit(&terminalFont.font);
    ffStrbufInit(&terminalFont.error);

    if(!ffDetectTerminalFont(instance, &terminalFont))
    {
        ffPrintError(instance, FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, "%s", terminalFont.error.chars);
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs.key);
            ffStrbufPutTo(&terminalFont.font.pretty, stdout);
        }
        else
        {
            ffPrintFormat(instance, FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_TERMINALFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont.font.pretty},
                {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont.font.name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &terminalFont.font.size},
                {FF_FORMAT_ARG_TYPE_LIST,   &terminalFont.font.styles}
            });
        }
    }

    ffStrbufDestroy(&terminalFont.error);
    ffFontDestroy(&terminalFont.font);
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

void ffParseTerminalFontJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFTerminalFontOptions __attribute__((__cleanup__(ffDestroyTerminalFontOptions))) options;
    ffInitTerminalFontOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_TERMINALFONT_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTerminalFont(instance, &options);
}
