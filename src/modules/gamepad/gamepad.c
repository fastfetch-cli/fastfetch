#include "common/printing.h"
#include "detection/gamepad/gamepad.h"
#include "modules/gamepad/gamepad.h"

#define FF_GAMEPAD_NUM_FORMAT_ARGS 2

static void printDevice(FFinstance* instance, FFGamepadOptions* options, const FFGamepadDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs.key);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_GAMEPAD_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->identifier},
        });
    }
}

void ffPrintGamepad(FFinstance* instance, FFGamepadOptions* options)
{
    FF_LIST_AUTO_DESTROY result;
    ffListInit(&result, sizeof(FFGamepadDevice));
    const char* error = ffDetectGamepad(instance, &result);

    if(error)
    {
        ffPrintError(instance, FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(instance, FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        printDevice(instance, options, device, result.length > 1 ? ++index : 0);
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}

void ffInitGamepadOptions(FFGamepadOptions* options)
{
    options->moduleName = FF_GAMEPAD_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseGamepadCommandOptions(FFGamepadOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_GAMEPAD_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyGamepadOptions(FFGamepadOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseGamepadJsonObject(FFinstance* instance, json_object* module)
{
    FFGamepadOptions __attribute__((__cleanup__(ffDestroyGamepadOptions))) options;
    ffInitGamepadOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_GAMEPAD_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintGamepad(instance, &options);
}
#endif
