#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/temps.h"
#include "detection/abi/abi.h"
#include "modules/abi/abi.h"
#include "util/stringUtils.h"

#define FF_ABI_NUM_FORMAT_ARGS 1

void ffPrintABI(FFABIOptions* options)
{
    FFABIResult result;
    ffListInit(&result.compats, sizeof(FFABICompat));
    ffListInit(&result.features, sizeof(FFABIFeature));

    const char* error = ffDetectABI(options, &result);
    if (error)
    {
        ffPrintError(FF_ABI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto out;
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    FF_LIST_FOR_EACH(FFABICompat, compat, result.compats)
    {
        if (options->moduleArgs.key.length == 0)
        {
            ffStrbufSetF(&buffer, FF_ABI_MODULE_NAME " (%s)", compat->name);
        }
        else
        {
            ffStrbufClear(&buffer);
            FF_PARSE_FORMAT_STRING_CHECKED(&buffer, &options->moduleArgs.key, 2, ((FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRING, compat->name, "name"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &options->moduleArgs.keyIcon, "icon"},
            }));
        }

        if (options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(buffer.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
            printf("%s\n", compat->desc);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_ABI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_ABI_NUM_FORMAT_ARGS, ((FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRING, compat->desc, "desc"},
            }));
        }
    }

out:
    ffListDestroy(&result.compats);
    ffListDestroy(&result.features);
}

bool ffParseABICommandOptions(FFABIOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_ABI_MODULE_NAME);
    if (!subKey) return false;
    return ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs);
}

void ffParseABIJsonObject(FFABIOptions* options, yyjson_val* module)
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

        ffPrintError(FF_ABI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateABIJsonConfig(FFABIOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyABIOptions))) FFABIOptions defaultOptions;
    ffInitABIOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateABIJsonResult(FFABIOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFABIResult result;
    ffListInit(&result.compats, sizeof(FFABICompat));
    ffListInit(&result.features, sizeof(FFABIFeature));

    const char* error = ffDetectABI(options, &result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto out;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    yyjson_mut_val* compats = yyjson_mut_obj_add_arr(doc, obj, "compats");
    FF_LIST_FOR_EACH(FFABICompat, compat, result.compats)
    {
        yyjson_mut_val* compatObj = yyjson_mut_arr_add_obj(doc, compats);
        yyjson_mut_obj_add_str(doc, compatObj, "name", compat->name);
        yyjson_mut_obj_add_str(doc, compatObj, "desc", compat->desc);
    }

    yyjson_mut_val* features = yyjson_mut_obj_add_arr(doc, obj, "features");
    FF_LIST_FOR_EACH(FFABIFeature, feat, result.features)
    {
        yyjson_mut_val* featObj = yyjson_mut_arr_add_obj(doc, features);
        yyjson_mut_obj_add_str(doc, featObj, "name", feat->name);
        yyjson_mut_obj_add_bool(doc, featObj, "supported", feat->supported);
    }

out:
    ffListDestroy(&result.compats);
    ffListDestroy(&result.features);
}

void ffPrintABIHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_ABI_MODULE_NAME, "{1}", FF_ABI_NUM_FORMAT_ARGS, ((const char* []) {
        "ABI Description - desc",
    }));
}

void ffInitABIOptions(FFABIOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_ABI_MODULE_NAME,
        "Print kernel ABI features and compatibility",
        ffParseABICommandOptions,
        ffParseABIJsonObject,
        ffPrintABI,
        ffGenerateABIJsonResult,
        ffPrintABIHelpFormat,
        ffGenerateABIJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyABIOptions(FFABIOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
