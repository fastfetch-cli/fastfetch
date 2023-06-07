#include "fastfetch.h"
#include "common/printing.h"
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
        ffPrintLogoAndKey(instance, FF_ICONS_MODULE_NAME, 0, &options->moduleArgs.key);
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

#ifdef FF_HAVE_JSONC
void ffParseIconsJsonObject(FFinstance* instance, json_object* module)
{
    FFIconsOptions __attribute__((__cleanup__(ffDestroyIconsOptions))) options;
    ffInitIconsOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintIcons(instance, &options);
}
#endif
