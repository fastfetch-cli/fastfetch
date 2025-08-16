#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/keyboard/keyboard.h"
#include "modules/keyboard/keyboard.h"
#include "util/stringUtils.h"

static void printDevice(FFKeyboardOptions* options, const FFKeyboardDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KEYBOARD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_KEYBOARD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->serial, "serial"),
        }));
    }
}

void ffPrintKeyboard(FFKeyboardOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFKeyboardDevice));

    const char* error = ffDetectKeyboard(&result);

    if(error)
    {
        ffPrintError(FF_KEYBOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_KEYBOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_LIST_FOR_EACH(FFKeyboardDevice, device, result)
    {
        printDevice(options, device, result.length > 1 ? ++index : 0);
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }
}

void ffParseKeyboardJsonObject(FFKeyboardOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_KEYBOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateKeyboardJsonConfig(FFKeyboardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

void ffGenerateKeyboardJsonResult(FF_MAYBE_UNUSED FFKeyboardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFKeyboardDevice));

    const char* error = ffDetectKeyboard(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFKeyboardDevice, device, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &device->serial);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &device->name);
    }

    FF_LIST_FOR_EACH(FFKeyboardDevice, device, result)
    {
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }
}

void ffInitKeyboardOptions(FFKeyboardOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyKeyboardOptions(FFKeyboardOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffKeyboardModuleInfo = {
    .name = FF_KEYBOARD_MODULE_NAME,
    .description = "List (connected) keyboards",
    .initOptions = (void*) ffInitKeyboardOptions,
    .destroyOptions = (void*) ffDestroyKeyboardOptions,
    .parseJsonObject = (void*) ffParseKeyboardJsonObject,
    .printModule = (void*) ffPrintKeyboard,
    .generateJsonResult = (void*) ffGenerateKeyboardJsonResult,
    .generateJsonConfig = (void*) ffGenerateKeyboardJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name", "name"},
        {"Serial number", "serial"},
    }))
};
