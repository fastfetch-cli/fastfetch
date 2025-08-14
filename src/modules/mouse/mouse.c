#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/mouse/mouse.h"
#include "modules/mouse/mouse.h"
#include "util/stringUtils.h"

static void printDevice(FFMouseOptions* options, const FFMouseDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_MOUSE_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_MOUSE_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->serial, "serial"),
        }));
    }
}

void ffPrintMouse(FFMouseOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFMouseDevice));

    const char* error = ffDetectMouse(&result);

    if(error)
    {
        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
    {
        printDevice(options, device, result.length > 1 ? ++index : 0);
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }
}

void ffParseMouseJsonObject(FFMouseOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateMouseJsonConfig(FFMouseOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

void ffGenerateMouseJsonResult(FF_MAYBE_UNUSED FFMouseOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFMouseDevice));

    const char* error = ffDetectMouse(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &device->serial);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &device->name);
    }

    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
    {
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }
}

void ffInitMouseOptions(FFMouseOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰍽");
}

void ffDestroyMouseOptions(FFMouseOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffMouseModuleInfo = {
    .name = FF_MOUSE_MODULE_NAME,
    .description = "List connected mouses",
    .initOptions = (void*) ffInitMouseOptions,
    .destroyOptions = (void*) ffDestroyMouseOptions,
    .parseJsonObject = (void*) ffParseMouseJsonObject,
    .printModule = (void*) ffPrintMouse,
    .generateJsonResult = (void*) ffGenerateMouseJsonResult,
    .generateJsonConfig = (void*) ffGenerateMouseJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Mouse name", "name"},
        {"Mouse serial number", "serial"},
    }))
};
