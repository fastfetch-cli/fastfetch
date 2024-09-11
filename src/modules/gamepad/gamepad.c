#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/gamepad/gamepad.h"
#include "modules/gamepad/gamepad.h"
#include "util/stringUtils.h"

#define FF_GAMEPAD_NUM_FORMAT_ARGS 4

static void printDevice(FFGamepadOptions* options, const FFGamepadDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateCopy(&device->name);

        if (device->battery > 0 && device->battery <= 100)
        {
            if (buffer.length)
                ffStrbufAppendC(&buffer, ' ');
            ffPercentAppendNum(&buffer, device->battery, options->percent, buffer.length > 0, &options->moduleArgs);
        }
        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY percentageNum = ffStrbufCreate();
        ffPercentAppendNum(&percentageNum, device->battery, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY percentageBar = ffStrbufCreate();
        ffPercentAppendBar(&percentageBar, device->battery, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_GAMEPAD_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->serial, "serial"),
            FF_FORMAT_ARG(percentageNum, "battery-percentage"),
            FF_FORMAT_ARG(percentageBar, "battery-percentage-bar"),
        }));
    }
}

void ffPrintGamepad(FFGamepadOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFGamepadDevice));

    const char* error = ffDetectGamepad(&result);

    if(error)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        printDevice(options, device, result.length > 1 ? ++index : 0);
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }
}

bool ffParseGamepadCommandOptions(FFGamepadOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_GAMEPAD_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
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

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateGamepadJsonConfig(FFGamepadOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyGamepadOptions))) FFGamepadOptions defaultOptions;
    ffInitGamepadOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
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

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &device->serial);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &device->name);
    }

    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }
}

void ffPrintGamepadHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_GAMEPAD_MODULE_NAME, "{1} ({3})", FF_GAMEPAD_NUM_FORMAT_ARGS, ((const char* []) {
        "Name - name",
        "Serial number - serial",
        "Battery percentage num - battery-percentage",
        "Battery percentage bar - battery-percentage-bar",
    }));
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
    ffOptionInitModuleArg(&options->moduleArgs, "󰺵");
    options->percent = (FFColorRangeConfig) { 50, 20 };
}

void ffDestroyGamepadOptions(FFGamepadOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
