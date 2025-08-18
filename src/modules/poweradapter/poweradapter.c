#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/poweradapter/poweradapter.h"
#include "modules/poweradapter/poweradapter.h"
#include "util/stringUtils.h"

#define FF_POWERADAPTER_DISPLAY_NAME "Power Adapter"

bool ffPrintPowerAdapter(FFPowerAdapterOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFPowerAdapterResult));

    const char* error = ffDetectPowerAdapter(&results);

    if (error)
    {
        ffPrintError(FF_POWERADAPTER_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if(results.length == 0)
    {
        ffPrintError(FF_POWERADAPTER_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No power adapters found");
        return false;
    }

    for(uint8_t i = 0; i < (uint8_t) results.length; i++)
    {
        FFPowerAdapterResult* result = FF_LIST_GET(FFPowerAdapterResult, results, i);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            if(result->name.length > 0)
                puts(result->name.chars);
            else
                printf("%dW\n", result->watts);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_POWERADAPTER_DISPLAY_NAME, i, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                FF_FORMAT_ARG(result->watts, "watts"),
                FF_FORMAT_ARG(result->name, "name"),
                FF_FORMAT_ARG(result->manufacturer, "manufacturer"),
                FF_FORMAT_ARG(result->modelName, "model-name"),
                FF_FORMAT_ARG(result->description, "description"),
                FF_FORMAT_ARG(result->serial, "serial"),
            }));
        }

        ffStrbufDestroy(&result->manufacturer);
        ffStrbufDestroy(&result->description);
        ffStrbufDestroy(&result->modelName);
        ffStrbufDestroy(&result->name);
        ffStrbufDestroy(&result->serial);
    }

    return true;
}

void ffGeneratePowerAdapterJsonConfig(FFPowerAdapterOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

void ffParsePowerAdapterJsonObject(FFPowerAdapterOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_POWERADAPTER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

bool ffGeneratePowerAdapterJsonResult(FF_MAYBE_UNUSED FFPowerAdapterOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFPowerAdapterResult));

    const char* error = ffDetectPowerAdapter(&results);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFPowerAdapterResult, item, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "description", &item->description);
        yyjson_mut_obj_add_strbuf(doc, obj, "manufacturer", &item->manufacturer);
        yyjson_mut_obj_add_strbuf(doc, obj, "modelName", &item->modelName);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &item->serial);
        yyjson_mut_obj_add_int(doc, obj, "watts", item->watts);
    }

    FF_LIST_FOR_EACH(FFPowerAdapterResult, item, results)
    {
        ffStrbufDestroy(&item->manufacturer);
        ffStrbufDestroy(&item->description);
        ffStrbufDestroy(&item->modelName);
        ffStrbufDestroy(&item->name);
        ffStrbufDestroy(&item->serial);
    }

    return true;
}

void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰚥");
}

void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffPowerAdapterModuleInfo = {
    .name = FF_POWERADAPTER_MODULE_NAME,
    .description = "Print power adapter name and charging watts",
    .initOptions = (void*) ffInitPowerAdapterOptions,
    .destroyOptions = (void*) ffDestroyPowerAdapterOptions,
    .parseJsonObject = (void*) ffParsePowerAdapterJsonObject,
    .printModule = (void*) ffPrintPowerAdapter,
    .generateJsonResult = (void*) ffGeneratePowerAdapterJsonResult,
    .generateJsonConfig = (void*) ffGeneratePowerAdapterJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Power adapter watts", "watts"},
        {"Power adapter name", "name"},
        {"Power adapter manufacturer", "manufacturer"},
        {"Power adapter model", "model"},
        {"Power adapter description", "description"},
        {"Power adapter serial number", "serial"},
    }))
};
