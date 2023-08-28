#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/poweradapter/poweradapter.h"
#include "modules/poweradapter/poweradapter.h"
#include "util/stringUtils.h"

#define FF_POWERADAPTER_DISPLAY_NAME "Power Adapter"
#define FF_POWERADAPTER_MODULE_ARGS 5

void ffPrintPowerAdapter(FFPowerAdapterOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(PowerAdapterResult));

    const char* error = ffDetectPowerAdapterImpl(&results);

    if (error)
    {
        ffPrintError(FF_POWERADAPTER_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
    }
    else if(results.length == 0)
    {
        ffPrintError(FF_POWERADAPTER_DISPLAY_NAME, 0, &options->moduleArgs, "No power adapters found");
    }
    else
    {
        for(uint8_t i = 0; i < (uint8_t) results.length; i++)
        {
            PowerAdapterResult* result = ffListGet(&results, i);

            if(result->watts != FF_POWERADAPTER_UNSET)
            {
                if(options->moduleArgs.outputFormat.length == 0)
                {
                    ffPrintLogoAndKey(FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

                    if(result->name.length > 0)
                        puts(result->name.chars);
                    else if(result->watts == FF_POWERADAPTER_NOT_CONNECTED)
                        puts("not connected");
                    else
                        printf("%dW\n", result->watts);
                }
                else
                {
                    ffPrintFormat(FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs, FF_POWERADAPTER_MODULE_ARGS, (FFformatarg[]){
                        {FF_FORMAT_ARG_TYPE_INT, &result->watts},
                        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
                        {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufacturer},
                        {FF_FORMAT_ARG_TYPE_STRBUF, &result->modelName},
                        {FF_FORMAT_ARG_TYPE_STRBUF, &result->description},
                    });
                }
            }

            ffStrbufDestroy(&result->manufacturer);
            ffStrbufDestroy(&result->description);
            ffStrbufDestroy(&result->modelName);
            ffStrbufDestroy(&result->name);
        }
    }
}

void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_POWERADAPTER_MODULE_NAME, ffParsePowerAdapterCommandOptions, ffParsePowerAdapterJsonObject, ffPrintPowerAdapter);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParsePowerAdapterCommandOptions(FFPowerAdapterOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_POWERADAPTER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParsePowerAdapterJsonObject(FFPowerAdapterOptions* options, yyjson_val* module)
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

        ffPrintError(FF_POWERADAPTER_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
