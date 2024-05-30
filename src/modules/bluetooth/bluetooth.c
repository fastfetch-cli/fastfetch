#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bluetooth/bluetooth.h"
#include "modules/bluetooth/bluetooth.h"
#include "util/stringUtils.h"

#define FF_BLUETOOTH_NUM_FORMAT_ARGS 5

static void printDevice(FFBluetoothOptions* options, const FFBluetoothResult* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateCopy(&device->name);

        if (device->battery > 0 && device->battery <= 100)
        {
            if (buffer.length)
                ffStrbufAppendC(&buffer, ' ');
            ffPercentAppendNum(&buffer, device->battery, options->percent, buffer.length > 0, &options->moduleArgs);
        }

        if (!device->connected)
            ffStrbufAppendS(&buffer, " [disconnected]");

        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY percentageStr = ffStrbufCreate();
        ffPercentAppendNum(&percentageStr, device->battery, options->percent, false, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_BLUETOOTH_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name, "name"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->address, "address"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->type, "type"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &percentageStr, "battery-percentage"},
            {FF_FORMAT_ARG_TYPE_BOOL, &device->connected, "connected"},
        }));
    }
}

void ffPrintBluetooth(FFBluetoothOptions* options)
{
    FF_LIST_AUTO_DESTROY devices = ffListCreate(sizeof (FFBluetoothResult));
    const char* error = ffDetectBluetooth(&devices);

    if(error)
    {
        ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    }
    else
    {
        FF_LIST_AUTO_DESTROY filtered = ffListCreate(sizeof(FFBluetoothResult*));

        FF_LIST_FOR_EACH(FFBluetoothResult, device, devices)
        {
            if(!device->connected && !options->showDisconnected)
                continue;

            *(FFBluetoothResult**)ffListAdd(&filtered) = device;
        }

        if(filtered.length == 0)
        {
            ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No bluetooth devices found");
        }

        for(uint32_t i = 0; i < filtered.length; i++)
        {
            uint8_t index = (uint8_t) (filtered.length == 1 ? 0 : i + 1);
            printDevice(options, *(FFBluetoothResult**)ffListGet(&filtered, i), index);
        }
    }

    FF_LIST_FOR_EACH(FFBluetoothResult, device, devices)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
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

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
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

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateBluetoothJsonConfig(FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBluetoothOptions))) FFBluetoothOptions defaultOptions;
    ffInitBluetoothOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->showDisconnected != defaultOptions.showDisconnected)
        yyjson_mut_obj_add_bool(doc, module, "showDisconnected", options->showDisconnected);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateBluetoothJsonResult(FF_MAYBE_UNUSED FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBluetoothResult));

    const char* error = ffDetectBluetooth(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFBluetoothResult, item, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "address", &item->address);
        yyjson_mut_obj_add_uint(doc, obj, "battery", item->battery);
        yyjson_mut_obj_add_bool(doc, obj, "connected", item->connected);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "type", &item->type);
    }

    FF_LIST_FOR_EACH(FFBluetoothResult, device, results)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
}

void ffPrintBluetoothHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_BLUETOOTH_MODULE_NAME, "{1} ({4})", FF_BLUETOOTH_NUM_FORMAT_ARGS, ((const char* []) {
        "Name - name",
        "Address - address",
        "Type - type",
        "Battery percentage - battery-percentage",
        "Is connected - connected",
    }));
}

void ffInitBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BLUETOOTH_MODULE_NAME,
        "List bluetooth devices",
        ffParseBluetoothCommandOptions,
        ffParseBluetoothJsonObject,
        ffPrintBluetooth,
        ffGenerateBluetoothJsonResult,
        ffPrintBluetoothHelpFormat,
        ffGenerateBluetoothJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    options->showDisconnected = false;
    options->percent = (FFColorRangeConfig) { 50, 20 };
}

void ffDestroyBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
