#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/font/font.h"
#include "modules/font/font.h"

#define FF_FONT_NUM_FORMAT_ARGS (FF_DETECT_FONT_NUM_FONTS + 1)

void ffPrintFont(FFinstance* instance, FFFontOptions* options)
{
    const FFFontResult* font = ffDetectFont(instance);

    if(font->error.length > 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &options->moduleArgs, "%s", font->error.chars);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_FONT_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufPutTo(&font->display, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_FONT_MODULE_NAME, 0, &options->moduleArgs, FF_FONT_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[0]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[1]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[2]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[3]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->display},
        });
    }
}

void ffInitFontOptions(FFFontOptions* options)
{
    options->moduleName = FF_FONT_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseFontCommandOptions(FFFontOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_FONT_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyFontOptions(FFFontOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseFontJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFFontOptions __attribute__((__cleanup__(ffDestroyFontOptions))) options;
    ffInitFontOptions(&options);

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

            ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintFont(instance, &options);
}
