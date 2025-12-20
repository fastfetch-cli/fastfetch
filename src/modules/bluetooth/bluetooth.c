#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bluetooth/bluetooth.h"
#include "modules/bluetooth/bluetooth.h"
#include "util/stringUtils.h"

static void printDevice(FFBluetoothOptions* options, const FFBluetoothResult* device, uint8_t index)
{
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        bool showBatteryLevel = device->battery > 0 && device->battery <= 100;

        if (showBatteryLevel && (percentType & FF_PERCENTAGE_TYPE_BAR_BIT))
        {
            ffPercentAppendBar(&buffer, device->battery, options->percent, &options->moduleArgs);
            ffStrbufAppendC(&buffer, ' ');
        }

        if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
            ffStrbufAppend(&buffer, &device->name);

        if (showBatteryLevel && (percentType & FF_PERCENTAGE_TYPE_NUM_BIT))
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
        FF_STRBUF_AUTO_DESTROY percentageNum = ffStrbufCreate();
        if(percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&percentageNum, device->battery, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY percentageBar = ffStrbufCreate();
        if(percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&percentageBar, device->battery, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(FF_BLUETOOTH_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->address, "address"),
            FF_FORMAT_ARG(device->type, "type"),
            FF_FORMAT_ARG(percentageNum, "battery-percentage"),
            FF_FORMAT_ARG(device->connected, "connected"),
            FF_FORMAT_ARG(percentageBar, "battery-percentage-bar"),
        }));
    }
}

bool ffPrintBluetooth(FFBluetoothOptions* options)
{
    FF_LIST_AUTO_DESTROY devices = ffListCreate(sizeof (FFBluetoothResult));
    const char* error = ffDetectBluetooth(options, &devices);

    if(error)
    {
        ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

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
        printDevice(options, *FF_LIST_GET(FFBluetoothResult*, filtered, i), index);
    }

    FF_LIST_FOR_EACH(FFBluetoothResult, device, devices)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
    return true;
}

void ffParseBluetoothJsonObject(FFBluetoothOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "showDisconnected"))
        {
            options->showDisconnected = yyjson_get_bool(val);
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_BLUETOOTH_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBluetoothJsonConfig(FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "showDisconnected", options->showDisconnected);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateBluetoothJsonResult(FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBluetoothResult));

    const char* error = ffDetectBluetooth(options, &results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
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
    return true;
}

void ffInitBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
    options->showDisconnected = false;
    options->percent = (FFPercentageModuleConfig) { 50, 20, 0 };
}

void ffDestroyBluetoothOptions(FFBluetoothOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBluetoothModuleInfo = {
    .name = FF_BLUETOOTH_MODULE_NAME,
    .description = "List (connected) bluetooth devices",
    .initOptions = (void*) ffInitBluetoothOptions,
    .destroyOptions = (void*) ffDestroyBluetoothOptions,
    .parseJsonObject = (void*) ffParseBluetoothJsonObject,
    .printModule = (void*) ffPrintBluetooth,
    .generateJsonResult = (void*) ffGenerateBluetoothJsonResult,
    .generateJsonConfig = (void*) ffGenerateBluetoothJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name", "name"},
        {"Address", "address"},
        {"Type", "type"},
        {"Battery percentage number", "battery-percentage"},
        {"Is connected", "connected"},
        {"Battery percentage bar", "battery-percentage-bar"},
    }))
};
