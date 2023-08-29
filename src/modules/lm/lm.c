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
        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(result.service.length == 0)
    {
        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, "No LM service found");
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
        ffPrintFormat(FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_LM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.service},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.type},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.version},
        });
    }
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.version);
}

void ffInitLMOptions(FFLMOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_LM_MODULE_NAME, ffParseLMCommandOptions, ffParseLMJsonObject, ffPrintLM, NULL);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseLMCommandOptions(FFLMOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyLMOptions(FFLMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
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

        ffPrintError(FF_LM_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
