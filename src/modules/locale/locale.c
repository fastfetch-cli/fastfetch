#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/locale/locale.h"
#include "modules/locale/locale.h"
#include "util/stringUtils.h"

#define FF_LOCALE_NUM_FORMAT_ARGS 1

void ffPrintLocale(FFinstance* instance, FFLocaleOptions* options)
{
    FF_STRBUF_AUTO_DESTROY locale = ffStrbufCreate();

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, "No locale found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufPutTo(&locale, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_LOCALE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &locale}
        });
    }
}

void ffInitLocaleOptions(FFLocaleOptions* options)
{
    options->moduleName = FF_LOCALE_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseLocaleCommandOptions(FFLocaleOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LOCALE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyLocaleOptions(FFLocaleOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseLocaleJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFLocaleOptions __attribute__((__cleanup__(ffDestroyLocaleOptions))) options;
    ffInitLocaleOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintLocale(instance, &options);
}
