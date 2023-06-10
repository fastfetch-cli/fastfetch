#include "fastfetch.h"
#include "common/printing.h"
#include "detection/poweradapter/poweradapter.h"
#include "modules/poweradapter/poweradapter.h"

#define FF_POWERADAPTER_DISPLAY_NAME "Power Adapter"
#define FF_POWERADAPTER_MODULE_ARGS 5

void ffPrintPowerAdapter(FFinstance* instance, FFPowerAdapterOptions* options)
{
    FFlist results;
    ffListInit(&results, sizeof(PowerAdapterResult));

    const char* error = ffDetectPowerAdapterImpl(instance, &results);

    if (error)
    {
        ffPrintError(instance, FF_POWERADAPTER_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
    }
    else if(results.length == 0)
    {
        ffPrintError(instance, FF_POWERADAPTER_DISPLAY_NAME, 0, &options->moduleArgs, "No power adapters found");
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
                    ffPrintLogoAndKey(instance, FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs.key);

                    if(result->name.length > 0)
                        puts(result->name.chars);
                    else if(result->watts == FF_POWERADAPTER_NOT_CONNECTED)
                        puts("not connected");
                    else
                        printf("%dW\n", result->watts);
                }
                else
                {
                    ffPrintFormat(instance, FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs, FF_POWERADAPTER_MODULE_ARGS, (FFformatarg[]){
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

    ffListDestroy(&results);
}

void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    options->moduleName = FF_POWERADAPTER_MODULE_NAME;
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

#ifdef FF_HAVE_JSONC
void ffParsePowerAdapterJsonObject(FFinstance* instance, json_object* module)
{
    FFPowerAdapterOptions __attribute__((__cleanup__(ffDestroyPowerAdapterOptions))) options;
    ffInitPowerAdapterOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_POWERADAPTER_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintPowerAdapter(instance, &options);
}
#endif
