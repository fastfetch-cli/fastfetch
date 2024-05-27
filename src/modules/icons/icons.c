#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/icons/icons.h"
#include "modules/icons/icons.h"
#include "util/stringUtils.h"

#define FF_ICONS_NUM_FORMAT_ARGS 1

void ffPrintIcons(FFIconsOptions* options)
{
    FF_STRBUF_AUTO_DESTROY icons = ffStrbufCreate();
    const char* error = ffDetectIcons(&icons);

    if(error)
    {
        ffPrintError(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&icons, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_ICONS_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &icons, "combined"}
        }));
    }
}

bool ffParseIconsCommandOptions(FFIconsOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_ICONS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseIconsJsonObject(FFIconsOptions* options, yyjson_val* module)
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

        ffPrintError(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateIconsJsonConfig(FFIconsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyIconsOptions))) FFIconsOptions defaultOptions;
    ffInitIconsOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateIconsJsonResult(FF_MAYBE_UNUSED FFIconsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY icons = ffStrbufCreate();
    const char* error = ffDetectIcons(&icons);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &icons);
}

void ffPrintIconsHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_ICONS_MODULE_NAME, "{1}", FF_ICONS_NUM_FORMAT_ARGS, ((const char* []) {
        "Combined icons - combined"
    }));
}

void ffInitIconsOptions(FFIconsOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_ICONS_MODULE_NAME,
        "Print icon style name",
        ffParseIconsCommandOptions,
        ffParseIconsJsonObject,
        ffPrintIcons,
        ffGenerateIconsJsonResult,
        ffPrintIconsHelpFormat,
        ffGenerateIconsJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyIconsOptions(FFIconsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
