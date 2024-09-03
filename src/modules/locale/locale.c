#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/locale/locale.h"
#include "modules/locale/locale.h"
#include "util/stringUtils.h"

#define FF_LOCALE_NUM_FORMAT_ARGS 1

void ffPrintLocale(FFLocaleOptions* options)
{
    FF_STRBUF_AUTO_DESTROY locale = ffStrbufCreate();

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        ffPrintError(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No locale found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&locale, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_LOCALE_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(locale, "result")
        }));
    }
}

bool ffParseLocaleCommandOptions(FFLocaleOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LOCALE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseLocaleJsonObject(FFLocaleOptions* options, yyjson_val* module)
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

        ffPrintError(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateLocaleJsonConfig(FFLocaleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyLocaleOptions))) FFLocaleOptions defaultOptions;
    ffInitLocaleOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateLocaleJsonResult(FF_MAYBE_UNUSED FFLocaleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY locale = ffStrbufCreate();

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No locale found");
        return;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &locale);
}

void ffPrintLocaleHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_LOCALE_MODULE_NAME, "{1}", FF_LOCALE_NUM_FORMAT_ARGS, ((const char* []) {
        "Locale code - result"
    }));
}

void ffInitLocaleOptions(FFLocaleOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_LOCALE_MODULE_NAME,
        "Print system locale name",
        ffParseLocaleCommandOptions,
        ffParseLocaleJsonObject,
        ffPrintLocale,
        ffGenerateLocaleJsonResult,
        ffPrintLocaleHelpFormat,
        ffGenerateLocaleJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyLocaleOptions(FFLocaleOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
