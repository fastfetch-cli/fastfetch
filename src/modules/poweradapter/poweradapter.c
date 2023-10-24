#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/poweradapter/poweradapter.h"
#include "modules/poweradapter/poweradapter.h"
#include "util/stringUtils.h"

#define FF_POWERADAPTER_DISPLAY_NAME "Power Adapter"
#define FF_POWERADAPTER_NUM_FORMAT_ARGS 5

void ffPrintPowerAdapter(FFPowerAdapterOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFPowerAdapterResult));

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
            FFPowerAdapterResult* result = ffListGet(&results, i);

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
                    ffPrintFormat(FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs, FF_POWERADAPTER_NUM_FORMAT_ARGS, (FFformatarg[]){
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

bool ffParsePowerAdapterCommandOptions(FFPowerAdapterOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_POWERADAPTER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffGeneratePowerAdapterJsonConfig(FFPowerAdapterOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyPowerAdapterOptions))) FFPowerAdapterOptions defaultOptions;
    ffInitPowerAdapterOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
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

void ffGeneratePowerAdapterJsonResult(FF_MAYBE_UNUSED FFPowerAdapterOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFPowerAdapterResult));

    const char* error = ffDetectPowerAdapterImpl(&results);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
    }
    else if(results.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No power adapters found");
    }
    else
    {
        yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
        FF_LIST_FOR_EACH(FFPowerAdapterResult, item, results)
        {
            yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
            yyjson_mut_obj_add_strbuf(doc, obj, "description", &item->description);
            yyjson_mut_obj_add_strbuf(doc, obj, "manufacturer", &item->manufacturer);
            yyjson_mut_obj_add_strbuf(doc, obj, "modelName", &item->modelName);
            yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
            yyjson_mut_obj_add_int(doc, obj, "watts", item->watts);
        }
    }
}

void ffPrintPowerAdapterHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_POWERADAPTER_MODULE_NAME, "{1}W", FF_POWERADAPTER_NUM_FORMAT_ARGS, (const char* []) {
        "PowerAdapter watts",
        "PowerAdapter name",
        "PowerAdapter manufacturer",
        "PowerAdapter model",
        "PowerAdapter description"
    });
}

void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_POWERADAPTER_MODULE_NAME,
        ffParsePowerAdapterCommandOptions,
        ffParsePowerAdapterJsonObject,
        ffPrintPowerAdapter,
        ffGeneratePowerAdapterJsonResult,
        ffPrintPowerAdapterHelpFormat,
        ffGeneratePowerAdapterJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
