#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bluetooth/bluetooth.h"
#include "modules/bluetooth/bluetooth.h"

static void printDevice(FFBluetoothOptions* options, const FFBluetoothResult* device, uint8_t index) {
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;
    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Bluetooth), index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        bool showBatteryLevel = device->battery > 0 && device->battery <= 100;

        if (showBatteryLevel && (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)) {
            ffPercentAppendBar(&buffer, device->battery, options->percent, &options->moduleArgs);
            ffStrbufAppendC(&buffer, ' ');
        }

        if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT)) {
            ffStrbufAppend(&buffer, &device->name);
        }

        if (showBatteryLevel && (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)) {
            if (buffer.length) {
                ffStrbufAppendC(&buffer, ' ');
            }
            ffPercentAppendNum(&buffer, device->battery, options->percent, buffer.length > 0, &options->moduleArgs);
        }

        if (!device->connected) {
            ffStrbufAppendS(&buffer, " [disconnected]");
        }

        ffStrbufPutTo(&buffer, stdout);
    } else {
        // The battery is `uint8_t` with 0 meaning "unknown", so the format string can only be filled
        // unconditionally; the signal quality has a sentinel of its own and is left empty instead.
        FF_STRBUF_AUTO_DESTROY batteryNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT) {
            ffPercentAppendNum(&batteryNum, device->battery, options->percent, false, &options->moduleArgs);
        }
        FF_STRBUF_AUTO_DESTROY batteryBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT) {
            ffPercentAppendBar(&batteryBar, device->battery, options->percent, &options->moduleArgs);
        }

        FF_STRBUF_AUTO_DESTROY signalNum = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY signalBar = ffStrbufCreate();
        if (device->signalQuality != -DBL_MAX) {
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT) {
                ffPercentAppendNum(&signalNum, device->signalQuality, options->percent, false, &options->moduleArgs);
            }
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT) {
                ffPercentAppendBar(&signalBar, device->signalQuality, options->percent, &options->moduleArgs);
            }
        }

        FF_LIST_AUTO_DESTROY deviceTypes = ffListCreate();
        if (device->deviceType & FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT) {
            ffStrbufInitStatic(FF_LIST_ADD(FFstrbuf, deviceTypes), "Classic");
        }
        if (device->deviceType & FF_BLUETOOTH_DEVICE_TYPE_LE_BIT) {
            ffStrbufInitStatic(FF_LIST_ADD(FFstrbuf, deviceTypes), "Low Energy");
        }

        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Bluetooth), index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                                  FF_ARG(device->name, "name"),
                                                                                                                  FF_ARG(device->address, "address"),
                                                                                                                  FF_ARG(device->type, "type"),
                                                                                                                  FF_ARG(deviceTypes, "device-type"),
                                                                                                                  FF_ARG(batteryNum, "battery-percentage"),
                                                                                                                  FF_ARG(batteryBar, "battery-percentage-bar"),
                                                                                                                  FF_ARG(signalNum, "signal-quality"),
                                                                                                                  FF_ARG(signalBar, "signal-quality-bar"),
                                                                                                                  FF_ARG(device->connected, "connected"),
                                                                                                              }));
    }
}

bool ffPrintBluetooth(FFBluetoothOptions* options) {
    FF_LIST_AUTO_DESTROY devices = ffListCreate();
    const char* error = ffDetectBluetooth(options, &devices);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Bluetooth), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (devices.length == 0) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Bluetooth), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No bluetooth devices found");
        return false;
    }

    uint8_t i = 1;
    FF_LIST_FOR_EACH (FFBluetoothResult, device, devices) {
        printDevice(options, device, devices.length == 1 ? 0 : i);
        ++i;
    }

    FF_LIST_FOR_EACH (FFBluetoothResult, device, devices) {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
    return true;
}

void ffParseBluetoothJsonObject(FFBluetoothOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showDisconnected")) {
            options->showDisconnected = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showType")) {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                                                                       { "classic", FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT },
                                                                       { "le", FF_BLUETOOTH_DEVICE_TYPE_LE_BIT },
                                                                       { "both", FF_BLUETOOTH_DEVICE_TYPE_BOTH },
                                                                       {},
                                                                   });
            if (error) {
                ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Bluetooth), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            } else {
                options->showType = (FFBluetoothDeviceType) value;
            }
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Bluetooth), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBluetoothJsonConfig(FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "showDisconnected", options->showDisconnected);

    switch (options->showType) {
        case FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT:
            yyjson_mut_obj_add_str(doc, module, "showType", "classic");
            break;
        case FF_BLUETOOTH_DEVICE_TYPE_LE_BIT:
            yyjson_mut_obj_add_str(doc, module, "showType", "le");
            break;
        case FF_BLUETOOTH_DEVICE_TYPE_BOTH:
            yyjson_mut_obj_add_str(doc, module, "showType", "both");
            break;
        case FF_BLUETOOTH_DEVICE_TYPE_NONE:
        default:
            break;
    }

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateBluetoothJsonResult(FFBluetoothOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_LIST_AUTO_DESTROY results = ffListCreate();

    const char* error = ffDetectBluetooth(options, &results);
    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH (FFBluetoothResult, item, results) {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "address", &item->address);
        yyjson_mut_obj_add_uint(doc, obj, "battery", item->battery);
        yyjson_mut_obj_add_bool(doc, obj, "connected", item->connected);

        yyjson_mut_val* deviceTypes = yyjson_mut_obj_add_arr(doc, obj, "deviceType");
        if (item->deviceType & FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT) {
            yyjson_mut_arr_add_str(doc, deviceTypes, "Classic");
        }
        if (item->deviceType & FF_BLUETOOTH_DEVICE_TYPE_LE_BIT) {
            yyjson_mut_arr_add_str(doc, deviceTypes, "Low Energy");
        }

        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        if (item->signalQuality != -DBL_MAX) {
            yyjson_mut_obj_add_real(doc, obj, "signalQuality", item->signalQuality);
        } else {
            yyjson_mut_obj_add_null(doc, obj, "signalQuality");
        }
        yyjson_mut_obj_add_strbuf(doc, obj, "type", &item->type);
    }

    FF_LIST_FOR_EACH (FFBluetoothResult, device, results) {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->address);
    }
    return true;
}

void ffInitBluetoothOptions(FFBluetoothOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");
    options->showDisconnected = false;
    // Both stacks are walked by default; `showType` exists to switch one off, not to switch one on.
    options->showType = FF_BLUETOOTH_DEVICE_TYPE_BOTH;
    options->percent = (FFPercentageModuleConfig) { 50, 20, 0 };
}

void ffDestroyBluetoothOptions(FFBluetoothOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBluetoothModuleInfo = {
    .name = "Bluetooth",
    .description = "List connected Bluetooth devices",
    .displayName = {
        .en = "Bluetooth",
        .ar = "بلوتوث",
        .cs = "Bluetooth",
        .de = "Bluetooth",
        .es = "Bluetooth",
        .fr = "Bluetooth",
        .gl = "Bluetooth",
        .he = "בלוטות'",
        .id = "Bluetooth",
        .it = "Bluetooth",
        .ja = "ブルートゥース",
        .ko = "블루투스",
        .pl = "Bluetooth",
        .pt = "Bluetooth",
        .ru = "Bluetooth",
        .tr = "Bluetooth",
        .uk = "Bluetooth",
        .vi = "Bluetooth",
        .zh_CN = "蓝牙设备",
        .zh_TW = "藍牙裝置",
    },
    .initOptions = (void*) ffInitBluetoothOptions,
    .destroyOptions = (void*) ffDestroyBluetoothOptions,
    .parseJsonObject = (void*) ffParseBluetoothJsonObject,
    .printModule = (void*) ffPrintBluetooth,
    .generateJsonResult = (void*) ffGenerateBluetoothJsonResult,
    .generateJsonConfig = (void*) ffGenerateBluetoothJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Name", "name" },
        { "Address", "address" },
        { "Type", "type" },
        { "Bluetooth stacks the device answers on", "device-type" },
        { "Battery percentage number", "battery-percentage" },
        { "Battery percentage bar", "battery-percentage-bar" },
        { "Signal quality number", "signal-quality" },
        { "Signal quality bar", "signal-quality-bar" },
        { "Is connected", "connected" },
    })),
    .defaultOrder = 59,
};
