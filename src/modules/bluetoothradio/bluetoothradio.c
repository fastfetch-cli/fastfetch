#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bluetoothradio/bluetoothradio.h"
#include "modules/bluetoothradio/bluetoothradio.h"
#include "util/stringUtils.h"

#define FF_BLUETOOTHRADIO_NUM_FORMAT_ARGS 8
#define FF_BLUETOOTHRADIO_DISPLAY_NAME "Bluetooth Radio"

static void printDevice(FFBluetoothRadioOptions* options, const FFBluetoothRadioResult* radio, uint8_t index)
{
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    if (options->moduleArgs.key.length == 0)
    {
        ffStrbufAppendF(&key, "%s (%s)", FF_BLUETOOTHRADIO_DISPLAY_NAME, radio->name.chars);
    }
    else
    {
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 3, ((FFformatarg[]){
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(radio->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    const char* version = NULL;

    switch (radio->lmpVersion < 0 ? -radio->lmpVersion : radio->lmpVersion)
    {
        case 0: version = "1.0b"; break;
        case 1: version = "1.1"; break;
        case 2: version = "1.2"; break;
        case 3: version = "2.0"; break;
        case 4: version = "2.1"; break;
        case 5: version = "3.0"; break;
        case 6: version = "4.0"; break;
        case 7: version = "4.1"; break;
        case 8: version = "4.2"; break;
        case 9: version = "5.0"; break;
        case 10: version = "5.1"; break;
        case 11: version = "5.2"; break;
        case 12: version = "5.3"; break;
        case 13: version = "5.4"; break;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        if (version)
            printf("Bluetooth %s%s (%s)\n", version, (radio->lmpVersion < 0 ? "+" : ""), radio->vendor.chars);
        else
            ffStrbufPutTo(&radio->vendor, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_BLUETOOTHRADIO_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(radio->name, "name"),
            FF_FORMAT_ARG(radio->address, "address"),
            FF_FORMAT_ARG(radio->lmpVersion, "lmp-version"),
            FF_FORMAT_ARG(radio->lmpSubversion, "lmp-subversion"),
            FF_FORMAT_ARG(version, "version"),
            FF_FORMAT_ARG(radio->vendor, "vendor"),
            FF_FORMAT_ARG(radio->discoverable, "discoverable"),
            FF_FORMAT_ARG(radio->connectable, "connectable"),
        }));
    }
}

void ffPrintBluetoothRadio(FFBluetoothRadioOptions* options)
{
    FF_LIST_AUTO_DESTROY radios = ffListCreate(sizeof (FFBluetoothRadioResult));
    const char* error = ffDetectBluetoothRadio(&radios);

    if(error)
    {
        ffPrintError(FF_BLUETOOTHRADIO_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    }
    else
    {
        uint8_t index = 0;
        FF_LIST_FOR_EACH(FFBluetoothRadioResult, radio, radios)
        {
            if (!radio->enabled)
                continue;

            index++;
            printDevice(options, radio, index);
        }

        if (index == 0)
        {
            if (radios.length > 0)
                ffPrintError(FF_BLUETOOTHRADIO_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Bluetooth radios found but none enabled");
            else
                ffPrintError(FF_BLUETOOTHRADIO_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No devices detected");
        }
    }

    FF_LIST_FOR_EACH(FFBluetoothRadioResult, radio, radios)
    {
        ffStrbufDestroy(&radio->name);
        ffStrbufDestroy(&radio->address);
        ffStrbufDestroy(&radio->vendor);
    }
}

bool ffParseBluetoothRadioCommandOptions(FFBluetoothRadioOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BLUETOOTHRADIO_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseBluetoothRadioJsonObject(FFBluetoothRadioOptions* options, yyjson_val* module)
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

        ffPrintError(FF_BLUETOOTHRADIO_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateBluetoothRadioJsonConfig(FFBluetoothRadioOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBluetoothRadioOptions))) FFBluetoothRadioOptions defaultOptions;
    ffInitBluetoothRadioOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateBluetoothRadioJsonResult(FF_MAYBE_UNUSED FFBluetoothRadioOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBluetoothRadioResult));

    const char* error = ffDetectBluetoothRadio(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFBluetoothRadioResult, item, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "address", &item->address);
        if (item->lmpVersion == INT_MIN)
            yyjson_mut_obj_add_null(doc, obj, "lmpVersion");
        else
            yyjson_mut_obj_add_int(doc, obj, "lmpVersion", item->lmpVersion);
        if (item->lmpSubversion == INT_MIN)
            yyjson_mut_obj_add_null(doc, obj, "lmpSubversion");
        else
            yyjson_mut_obj_add_int(doc, obj, "lmpSubversion", item->lmpSubversion);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &item->vendor);
        yyjson_mut_obj_add_bool(doc, obj, "enabled", item->enabled);
        yyjson_mut_obj_add_bool(doc, obj, "discoverable", item->discoverable);
        yyjson_mut_obj_add_bool(doc, obj, "connectable", item->connectable);
    }

    FF_LIST_FOR_EACH(FFBluetoothRadioResult, radio, results)
    {
        ffStrbufDestroy(&radio->name);
        ffStrbufDestroy(&radio->address);
        ffStrbufDestroy(&radio->vendor);
    }
}

void ffPrintBluetoothRadioHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_BLUETOOTHRADIO_MODULE_NAME, "Bluetooth {5} ({6})", FF_BLUETOOTHRADIO_NUM_FORMAT_ARGS, ((const char* []) {
        "Radio name for discovering - name",
        "Address - local radio address",
        "LMP version - lmp-version",
        "LMP subversion - lmp-subversion",
        "Bluetooth version - version",
        "Vendor - vendor",
        "Discoverable - discoverable",
        "Connectable / Pairable - connectable",
    }));
}

void ffInitBluetoothRadioOptions(FFBluetoothRadioOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BLUETOOTHRADIO_MODULE_NAME,
        "List bluetooth radios width supported version and vendor",
        ffParseBluetoothRadioCommandOptions,
        ffParseBluetoothRadioJsonObject,
        ffPrintBluetoothRadio,
        ffGenerateBluetoothRadioJsonResult,
        ffPrintBluetoothRadioHelpFormat,
        ffGenerateBluetoothRadioJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰐻");
}

void ffDestroyBluetoothRadioOptions(FFBluetoothRadioOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
