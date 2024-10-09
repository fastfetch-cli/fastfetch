#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/keyboard/keyboard.h"
#include "modules/keyboard/keyboard.h"
#include "util/stringUtils.h"

#define FF_KEYBOARD_NUM_FORMAT_ARGS 2

static void printDevice(FFKeyboardOptions* options, const FFKeyboardDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KEYBOARD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_KEYBOARD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_KEYBOARD_NUM_FORMAT_ARGS, ((FFformatarg[]) {
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

bool ffParseKeyboardCommandOptions(FFKeyboardOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_KEYBOARD_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseKeyboardJsonObject(FFKeyboardOptions* options, yyjson_val* module)
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

        ffPrintError(FF_KEYBOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateKeyboardJsonConfig(FFKeyboardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyKeyboardOptions))) FFKeyboardOptions defaultOptions;
    ffInitKeyboardOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
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

void ffPrintKeyboardHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_KEYBOARD_MODULE_NAME, "{1} ({3})", FF_KEYBOARD_NUM_FORMAT_ARGS, ((const char* []) {
        "Name - name",
        "Serial number - serial",
    }));
}

void ffInitKeyboardOptions(FFKeyboardOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_KEYBOARD_MODULE_NAME,
        "List connected keyboards",
        ffParseKeyboardCommandOptions,
        ffParseKeyboardJsonObject,
        ffPrintKeyboard,
        ffGenerateKeyboardJsonResult,
        ffPrintKeyboardHelpFormat,
        ffGenerateKeyboardJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyKeyboardOptions(FFKeyboardOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
