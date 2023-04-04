#include "fastfetch.h"
#include "common/printing.h"
#include "detection/locale/locale.h"
#include "modules/locale/locale.h"

#define FF_LOCALE_NUM_FORMAT_ARGS 1

void ffPrintLocale(FFinstance* instance, FFLocaleOptions* options)
{
    FF_STRBUF_AUTO_DESTROY locale;
    ffStrbufInit(&locale);

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, "No locale found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs.key);
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

#ifdef FF_HAVE_JSONC
void ffParseLocaleJsonObject(FFinstance* instance, json_object* module)
{
    FFLocaleOptions __attribute__((__cleanup__(ffDestroyLocaleOptions))) options;
    ffInitLocaleOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintLocale(instance, &options);
}
#endif
