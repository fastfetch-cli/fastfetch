#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/lm/lm.h"
#include "modules/lm/lm.h"
#include "util/stringUtils.h"

#define FF_LM_NUM_FORMAT_ARGS 3

void ffPrintLM(FFLMOptions* options)
{
    FFLMResult result;
    ffStrbufInit(&result.service);
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.version);
    const char* error = ffDetectLM(&result);

    if(error)
    {
        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(result.service.length == 0)
    {
        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No LM service found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.service, stdout);
        if(result.version.length)
            printf(" %s", result.version.chars);
        if(result.type.length)
            printf(" (%s)", result.type.chars);
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_LM_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.service, "service"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.type, "type"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.version, "version"},
        }));
    }
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.version);
}

bool ffParseLMCommandOptions(FFLMOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseLMJsonObject(FFLMOptions* options, yyjson_val* module)
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

        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateLMJsonConfig(FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyLMOptions))) FFLMOptions defaultOptions;
    ffInitLMOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateLMJsonResult(FF_MAYBE_UNUSED FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFLMResult result;
    ffStrbufInit(&result.service);
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.version);
    const char* error = ffDetectLM(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if(result.service.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No LM service found");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "service", &result.service);
    yyjson_mut_obj_add_strbuf(doc, obj, "type", &result.type);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);

exit:
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.version);
}

void ffPrintLMHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_LM_MODULE_NAME, "{1} {3} ({2})", FF_LM_NUM_FORMAT_ARGS, ((const char* []) {
        "LM service - service",
        "LM type - type",
        "LM version - version"
    }));
}

void ffInitLMOptions(FFLMOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_LM_MODULE_NAME,
        "Print login manager (desktop manager) name and version",
        ffParseLMCommandOptions,
        ffParseLMJsonObject,
        ffPrintLM,
        ffGenerateLMJsonResult,
        ffPrintLMHelpFormat,
        ffGenerateLMJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyLMOptions(FFLMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
