#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/gamepad/gamepad.h"
#include "modules/gamepad/gamepad.h"
#include "util/stringUtils.h"

static void printDevice(FFGamepadOptions* options, const FFGamepadDevice* device, uint8_t index)
{
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

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
        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY percentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&percentageNum, device->battery, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY percentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&percentageBar, device->battery, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(FF_GAMEPAD_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->serial, "serial"),
            FF_FORMAT_ARG(percentageNum, "battery-percentage"),
            FF_FORMAT_ARG(percentageBar, "battery-percentage-bar"),
        }));
    }
}

bool ffPrintGamepad(FFGamepadOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFGamepadDevice));

    const char* error = ffDetectGamepad(&result);

    if(error)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if(!result.length)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No devices detected");
        return false;
    }

    FF_LIST_AUTO_DESTROY filtered = ffListCreate(sizeof(FFGamepadDevice*));
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        bool ignored = false;
        FF_LIST_FOR_EACH(FFstrbuf, ignore, options->ignores)
        {
            if(ffStrbufStartsWithIgnCase(&device->name, ignore))
            {
                ignored = true;
                break;
            }
        }
        if(!ignored)
        {
            FFGamepadDevice** ptr = ffListAdd(&filtered);
            *ptr = device;
        }
    }

    bool ret = true;
    if(!filtered.length)
    {
        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "All devices are ignored");
        ret = false;
    }
    else
    {
        uint8_t index = 0;
        FF_LIST_FOR_EACH(FFGamepadDevice*, pdevice, filtered)
        {
            FFGamepadDevice* device = *pdevice;
            printDevice(options, device, filtered.length > 1 ? ++index : 0);
        }

        FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
        {
            ffStrbufDestroy(&device->serial);
            ffStrbufDestroy(&device->name);
        }
    }

    return ret;
}

void ffParseGamepadJsonObject(FFGamepadOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "ignores"))
        {
            yyjson_val *elem;
            size_t eidx, emax;
            yyjson_arr_foreach(val, eidx, emax, elem)
            {
                if (yyjson_is_str(elem))
                {
                    FFstrbuf* strbuf = ffListAdd(&options->ignores);
                    ffStrbufInitJsonVal(strbuf, elem);
                }
            }
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_GAMEPAD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateGamepadJsonConfig(FFGamepadOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    if (options->ignores.length > 0)
    {
        yyjson_mut_val* ignores = yyjson_mut_obj_add_arr(doc, module, "ignores");
        FF_LIST_FOR_EACH(FFstrbuf, strbuf, options->ignores)
            yyjson_mut_arr_append(ignores, yyjson_mut_strncpy(doc, strbuf->chars, strbuf->length));
    }
    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateGamepadJsonResult(FF_MAYBE_UNUSED FFGamepadOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFGamepadDevice));

    const char* error = ffDetectGamepad(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &device->serial);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &device->name);

        bool ignored = false;
        FF_LIST_FOR_EACH(FFstrbuf, ignore, options->ignores)
        {
            if(ffStrbufStartsWithIgnCase(&device->name, ignore))
            {
                ignored = true;
                break;
            }
        }
        yyjson_mut_obj_add_bool(doc, obj, "ignored", ignored);
    }

    FF_LIST_FOR_EACH(FFGamepadDevice, device, result)
    {
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }

    return true;
}

void ffInitGamepadOptions(FFGamepadOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰺵");

    ffListInit(&options->ignores, sizeof(FFstrbuf));
    options->percent = (FFPercentageModuleConfig) { 50, 20, 0 };
}

void ffDestroyGamepadOptions(FFGamepadOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    FF_LIST_FOR_EACH(FFstrbuf, str, options->ignores)
        ffStrbufDestroy(str);
    ffListDestroy(&options->ignores);
}

FFModuleBaseInfo ffGamepadModuleInfo = {
    .name = FF_GAMEPAD_MODULE_NAME,
    .description = "List (connected) gamepads",
    .initOptions = (void*) ffInitGamepadOptions,
    .destroyOptions = (void*) ffDestroyGamepadOptions,
    .parseJsonObject = (void*) ffParseGamepadJsonObject,
    .printModule = (void*) ffPrintGamepad,
    .generateJsonResult = (void*) ffGenerateGamepadJsonResult,
    .generateJsonConfig = (void*) ffGenerateGamepadJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name", "name"},
        {"Serial number", "serial"},
        {"Battery percentage num", "battery-percentage"},
        {"Battery percentage bar", "battery-percentage-bar"},
    }))
};
