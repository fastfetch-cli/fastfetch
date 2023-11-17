#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/gamepad/gamepad.h"
#include "modules/gamepad/gamepad.h"
#include "util/stringUtils.h"

#define FF_GAMEPAD_NUM_FORMAT_ARGS 2

static void printDevice(FFGamepadOptions* options, const FFGamepadDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        ffPrintFormat(FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_GAMEPAD_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->identifier},
        });
    }
}

void ffPrintGamepad(FFGamepadOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFGamepadDevice));

    const char* error = ffDetectGamepad(&result);

    if(error)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        printDevice(options, device, result.length > 1 ? ++index : 0);
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}

bool ffParseGamepadCommandOptions(FFGamepadOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_GAMEPAD_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseGamepadJsonObject(FFGamepadOptions* options, yyjson_val* module)
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

        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateGamepadJsonConfig(FFGamepadOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyGamepadOptions))) FFGamepadOptions defaultOptions;
    ffInitGamepadOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateGamepadJsonResult(FF_MAYBE_UNUSED FFGamepadOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFGamepadDevice));

    const char* error = ffDetectGamepad(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    if(!result.length)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No devices detected");
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "identifier", &device->identifier);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &device->name);
    }

    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}

void ffPrintGamepadHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_GAMEPAD_MODULE_NAME, "{1}", FF_GAMEPAD_NUM_FORMAT_ARGS, (const char* []) {
        "Name",
        "Identifier"
    });
}

void ffInitGamepadOptions(FFGamepadOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_GAMEPAD_MODULE_NAME,
        "List connected gamepads",
        ffParseGamepadCommandOptions,
        ffParseGamepadJsonObject,
        ffPrintGamepad,
        ffGenerateGamepadJsonResult,
        ffPrintGamepadHelpFormat,
        ffGenerateGamepadJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyGamepadOptions(FFGamepadOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
