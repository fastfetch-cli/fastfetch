#include "common/printing.h"
#include "detection/gamepad/gamepad.h"

#define FF_GAMEPAD_MODULE_NAME "Gamepad"
#define FF_GAMEPAD_NUM_FORMAT_ARGS 2

static void printDevice(FFinstance* instance, const FFGamepadDevice* device, uint8_t index)
{
    if(instance->config.gamepad.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_GAMEPAD_MODULE_NAME, index, &instance->config.gamepad.key);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_GAMEPAD_MODULE_NAME, index, &instance->config.gamepad, FF_GAMEPAD_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->identifier},
        });
    }
}

void ffPrintGamepad(FFinstance* instance)
{
    FF_LIST_AUTO_DESTROY result;
    ffListInit(&result, sizeof(FFGamepadDevice));
    const char* error = ffDetectGamepad(instance, &result);

    if(error)
    {
        ffPrintError(instance, FF_GAMEPAD_MODULE_NAME, 0, &instance->config.gamepad, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(instance, FF_GAMEPAD_MODULE_NAME, 0, &instance->config.gamepad, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        printDevice(instance, device, result.length > 1 ? ++index : 0);
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}
