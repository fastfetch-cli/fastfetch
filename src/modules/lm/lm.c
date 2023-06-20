#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/lm/lm.h"
#include "modules/lm/lm.h"
#include "util/stringUtils.h"

#define FF_LM_NUM_FORMAT_ARGS 2

void ffPrintLM(FFinstance* instance, FFLMOptions* options)
{
    FFLMResult result;
    const char* error = ffDetectLM(&result);

    if(error)
    {
        ffPrintError(instance, FF_LM_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(result.service.length == 0)
    {
        ffPrintError(instance, FF_LM_MODULE_NAME, 0, &options->moduleArgs, "No LM service found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_LM_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufWriteTo(&result.service, stdout);
        if(result.type.length)
            printf(" (%s)", result.type.chars);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_LM_MODULE_NAME, 0, &options->moduleArgs, FF_LM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.service},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.type},
        });
    }
}

void ffInitLMOptions(FFLMOptions* options)
{
    options->moduleName = FF_LM_MODULE_NAME;
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

void ffParseLMJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFLMOptions __attribute__((__cleanup__(ffDestroyLMOptions))) options;
    ffInitLMOptions(&options);

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

            ffPrintError(instance, FF_LM_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintLM(instance, &options);
}
