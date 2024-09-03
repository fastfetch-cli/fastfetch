#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/icons/icons.h"
#include "modules/icons/icons.h"
#include "util/stringUtils.h"

#define FF_ICONS_NUM_FORMAT_ARGS 2

void ffPrintIcons(FFIconsOptions* options)
{
    FFIconsResult result = {
        .icons1 = ffStrbufCreate(),
        .icons2 = ffStrbufCreate(),
    };
    const char* error = ffDetectIcons(&result);

    if(error)
    {
        ffPrintError(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        if (result.icons1.length)
            ffStrbufWriteTo(&result.icons1, stdout);
        if (result.icons2.length)
        {
            if (result.icons1.length)
                fputs(", ", stdout);
            ffStrbufWriteTo(&result.icons2, stdout);
        }
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_ICONS_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(result.icons1, "icons1"),
            FF_FORMAT_ARG(result.icons2, "icons2"),
        }));
    }

    ffStrbufDestroy(&result.icons1);
    ffStrbufDestroy(&result.icons2);
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
    FFIconsResult result = {
        .icons1 = ffStrbufCreate(),
        .icons2 = ffStrbufCreate()
    };
    const char* error = ffDetectIcons(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* icons = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, icons, "icons1", &result.icons1);
    yyjson_mut_obj_add_strbuf(doc, icons, "icons2", &result.icons2);

    ffStrbufDestroy(&result.icons1);
    ffStrbufDestroy(&result.icons2);
}

void ffPrintIconsHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_ICONS_MODULE_NAME, "{1}, {2}", FF_ICONS_NUM_FORMAT_ARGS, ((const char* []) {
        "Icons part 1 - icons1",
        "Icons part 2 - icons2",
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
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyIconsOptions(FFIconsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
