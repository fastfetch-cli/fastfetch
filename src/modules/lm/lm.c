#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/lm/lm.h"
#include "modules/lm/lm.h"
#include "util/stringUtils.h"

bool ffPrintLM(FFLMOptions* options)
{
    bool success = false;
    FFLMResult result;
    ffStrbufInit(&result.service);
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.version);
    const char* error = ffDetectLM(&result);

    if(error)
    {
        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(result.service.length == 0)
    {
        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No LM service found");
        goto exit;
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
        FF_PRINT_FORMAT_CHECKED(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result.service, "service"),
            FF_FORMAT_ARG(result.type, "type"),
            FF_FORMAT_ARG(result.version, "version"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.version);

    return success;
}

void ffParseLMJsonObject(FFLMOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateLMJsonConfig(FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateLMJsonResult(FF_MAYBE_UNUSED FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
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
    success = true;

exit:
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.version);

    return success;
}

void ffInitLMOptions(FFLMOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰧨");
}

void ffDestroyLMOptions(FFLMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffLMModuleInfo = {
    .name = FF_LM_MODULE_NAME,
    .description = "Print login manager (desktop manager) name and version",
    .initOptions = (void*) ffInitLMOptions,
    .destroyOptions = (void*) ffDestroyLMOptions,
    .parseJsonObject = (void*) ffParseLMJsonObject,
    .printModule = (void*) ffPrintLM,
    .generateJsonResult = (void*) ffGenerateLMJsonResult,
    .generateJsonConfig = (void*) ffGenerateLMJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"LM service", "service"},
        {"LM type", "type"},
        {"LM version", "version"},
    }))
};
