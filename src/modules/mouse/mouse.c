#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/mouse/mouse.h"
#include "modules/mouse/mouse.h"
#include "util/stringUtils.h"

#define FF_MOUSE_NUM_FORMAT_ARGS 2

static void printDevice(FFMouseOptions* options, const FFMouseDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_MOUSE_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_MOUSE_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_MOUSE_NUM_FORMAT_ARGS, ((FFformatarg[]) {
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

bool ffParseMouseCommandOptions(FFMouseOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_MOUSE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseMouseJsonObject(FFMouseOptions* options, yyjson_val* module)
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

        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateMouseJsonConfig(FFMouseOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyMouseOptions))) FFMouseOptions defaultOptions;
    ffInitMouseOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
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

void ffPrintMouseHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_MOUSE_MODULE_NAME, "{1} ({3})", FF_MOUSE_NUM_FORMAT_ARGS, ((const char* []) {
        "Name - name",
        "Serial number - serial",
    }));
}

void ffInitMouseOptions(FFMouseOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_MOUSE_MODULE_NAME,
        "List connected mouses",
        ffParseMouseCommandOptions,
        ffParseMouseJsonObject,
        ffPrintMouse,
        ffGenerateMouseJsonResult,
        ffPrintMouseHelpFormat,
        ffGenerateMouseJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰍽");
}

void ffDestroyMouseOptions(FFMouseOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
