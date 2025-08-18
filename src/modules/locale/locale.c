#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/locale/locale.h"
#include "modules/locale/locale.h"
#include "util/stringUtils.h"

bool ffPrintLocale(FFLocaleOptions* options)
{
    FF_STRBUF_AUTO_DESTROY locale = ffStrbufCreate();

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        ffPrintError(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No locale found");
        return false;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&locale, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(locale, "result")
        }));
    }

    return true;
}

void ffParseLocaleJsonObject(FFLocaleOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_LOCALE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateLocaleJsonConfig(FFLocaleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateLocaleJsonResult(FF_MAYBE_UNUSED FFLocaleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY locale = ffStrbufCreate();

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No locale found");
        return false;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &locale);

    return true;
}

void ffInitLocaleOptions(FFLocaleOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyLocaleOptions(FFLocaleOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffLocaleModuleInfo = {
    .name = FF_LOCALE_MODULE_NAME,
    .description = "Print system locale name",
    .initOptions = (void*) ffInitLocaleOptions,
    .destroyOptions = (void*) ffDestroyLocaleOptions,
    .parseJsonObject = (void*) ffParseLocaleJsonObject,
    .printModule = (void*) ffPrintLocale,
    .generateJsonResult = (void*) ffGenerateLocaleJsonResult,
    .generateJsonConfig = (void*) ffGenerateLocaleJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Locale code", "result"},
    }))
};
