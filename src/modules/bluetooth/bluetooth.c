#include "common/printing.h"
#include "detection/bluetooth/bluetooth.h"
#include "modules/bluetooth/bluetooth.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"
#define FF_BLUETOOTH_NUM_FORMAT_ARGS 4

static void printDevice(FFinstance* instance, FFBluetoothOptions* options, const FFBluetoothDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs.key);
        ffStrbufWriteTo(&device->name, stdout);

        if(device->battery > 0)
            printf(" (%d%%)", device->battery);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_BLUETOOTH_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->address},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->type},
            {FF_FORMAT_ARG_TYPE_UINT8, &device->battery}
        });
    }
}

void ffPrintBluetooth(FFinstance* instance, FFBluetoothOptions* options)
{
    const FFBluetoothResult* bluetooth = ffDetectBluetooth(instance);

    if(bluetooth->error.length > 0)
    {
        ffPrintError(instance, FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, "%s", bluetooth->error.chars);
        return;
    }

    FFlist filtered;
    ffListInit(&filtered, sizeof(FFBluetoothDevice*));

    FF_LIST_FOR_EACH(FFBluetoothDevice, device, bluetooth->devices)
    {
        if(!device->connected && !options->showDisconnected)
            continue;

        *(FFBluetoothDevice**)ffListAdd(&filtered) = device;
    }

    if(filtered.length == 0)
    {
        ffPrintError(instance, FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, "No bluetooth devices found");
        return;
    }

    for(uint32_t i = 0; i < filtered.length; i++)
    {
        uint8_t index = (uint8_t) (filtered.length == 1 ? 0 : i + 1);
        printDevice(instance, options, *(FFBluetoothDevice**)ffListGet(&filtered, i), index);
    }

    ffListDestroy(&filtered);
}

void ffInitBluetoothOptions(FFBluetoothOptions* options)
{
    options->moduleName = FF_BLUETOOTH_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
    options->showDisconnected = false;
}

bool ffParseBluetoothCommandOptions(FFBluetoothOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BLUETOOTH_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "show-disconnected") == 0)
        options->showDisconnected = ffOptionParseBoolean(value);
    return false;
}

void ffDestroyBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
bool ffParseBluetoothJsonObject(FFinstance* instance, const char* type, json_object* module)
{
    if (strcasecmp(type, FF_BLUETOOTH_MODULE_NAME) != 0)
        return false;

    FFBluetoothOptions __attribute__((__cleanup__(ffDestroyBluetoothOptions))) options;
    ffInitBluetoothOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "showConnected") == 0)
            {
                options.showDisconnected = json_object_get_boolean(val);
                continue;
            }

            ffPrintError(instance, FF_BLUETOOTH_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintBluetooth(instance, &options);
    return true;
}
#endif
