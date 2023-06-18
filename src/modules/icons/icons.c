#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/icons/icons.h"
#include "modules/icons/icons.h"

#define FF_ICONS_NUM_FORMAT_ARGS 1

void ffPrintIcons(FFinstance* instance, FFIconsOptions* options)
{
    FF_STRBUF_AUTO_DESTROY icons = ffStrbufCreate();
    const char* error = ffDetectIcons(instance, &icons);

    if(error)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_ICONS_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufPutTo(&icons, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_ICONS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &icons}
        });
    }
}

void ffInitIconsOptions(FFIconsOptions* options)
{
    options->moduleName = FF_ICONS_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseIconsCommandOptions(FFIconsOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_ICONS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyIconsOptions(FFIconsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseIconsJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFIconsOptions __attribute__((__cleanup__(ffDestroyIconsOptions))) options;
    ffInitIconsOptions(&options);

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

            ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintIcons(instance, &options);
}
