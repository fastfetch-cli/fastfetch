#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bluetooth/bluetooth.h"
#include "modules/bluetooth/bluetooth.h"
#include "util/stringUtils.h"

#define FF_BLUETOOTH_NUM_FORMAT_ARGS 4

static void printDevice(FFBluetoothOptions* options, const FFBluetoothDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&device->name, stdout);

        if(device->battery > 0)
            printf(" (%d%%)", device->battery);

        if(!device->connected)
            puts(" [disconnected]");
        else
            putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_BLUETOOTH_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->address},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->type},
            {FF_FORMAT_ARG_TYPE_UINT8, &device->battery}
        });
    }
}

void ffPrintBluetooth(FFBluetoothOptions* options)
{
    FF_LIST_AUTO_DESTROY devices = ffListCreate(sizeof (FFBluetoothDevice));
    const char* error = ffDetectBluetooth(&devices);

    if(error)
    {
        ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
    }
    else
    {
        FF_LIST_AUTO_DESTROY filtered = ffListCreate(sizeof(FFBluetoothDevice*));

        FF_LIST_FOR_EACH(FFBluetoothDevice, device, devices)
        {
            if(!device->connected && !options->showDisconnected)
                continue;

            *(FFBluetoothDevice**)ffListAdd(&filtered) = device;
        }

        if(filtered.length == 0)
        {
            ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, "No bluetooth devices found");
        }

        for(uint32_t i = 0; i < filtered.length; i++)
        {
            uint8_t index = (uint8_t) (filtered.length == 1 ? 0 : i + 1);
            printDevice(options, *(FFBluetoothDevice**)ffListGet(&filtered, i), index);
        }
    }

    FF_LIST_FOR_EACH(FFBluetoothDevice, device, devices)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
}

void ffInitBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_BLUETOOTH_MODULE_NAME, ffParseBluetoothCommandOptions, ffParseBluetoothJsonObject, ffPrintBluetooth, ffGenerateBluetoothJson);
    ffOptionInitModuleArg(&options->moduleArgs);
    options->showDisconnected = false;
}

bool ffParseBluetoothCommandOptions(FFBluetoothOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BLUETOOTH_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "show-disconnected"))
    {
        options->showDisconnected = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseBluetoothJsonObject(FFBluetoothOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "showDisconnected"))
        {
            options->showDisconnected = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateBluetoothJson(FF_MAYBE_UNUSED FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBluetoothDevice));

    const char* error = ffDetectBluetooth(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }
    else
    {
        yyjson_mut_val* arr = yyjson_mut_arr(doc);
        yyjson_mut_obj_add_val(doc, module, "result", arr);

        FF_LIST_FOR_EACH(FFBluetoothDevice, item, results)
        {
            yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
            yyjson_mut_obj_add_strbuf(doc, obj, "address", &item->address);
            yyjson_mut_obj_add_uint(doc, obj, "battery", item->battery);
            yyjson_mut_obj_add_bool(doc, obj, "connected", item->connected);
            yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
            yyjson_mut_obj_add_strbuf(doc, obj, "type", &item->type);
        }
    }

    FF_LIST_FOR_EACH(FFBluetoothDevice, device, results)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
}
