#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/mouse/mouse.h"
#include "modules/mouse/mouse.h"
#include "util/stringUtils.h"

static void printDevice(FFMouseOptions* options, const FFMouseDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_MOUSE_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&device->name, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_MOUSE_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->serial, "serial"),
        }));
    }
}

bool ffPrintMouse(FFMouseOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFMouseDevice));

    const char* error = ffDetectMouse(&result);

    if(error)
    {
        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if(!result.length)
    {
        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No devices detected");
        return false;
    }

    FF_LIST_AUTO_DESTROY filtered = ffListCreate(sizeof(FFMouseDevice*));
    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
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
            FFMouseDevice** ptr = ffListAdd(&filtered);
            *ptr = device;
        }
    }

    bool ret = true;
    if(!filtered.length)
    {
        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "All devices are ignored");
        ret = false;
    }
    else
    {
        uint8_t index = 0;
        FF_LIST_FOR_EACH(FFMouseDevice*, pdevice, filtered)
        {
            FFMouseDevice* device = *pdevice;
            printDevice(options, device, filtered.length > 1 ? ++index : 0);
        }
    }

    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
    {
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }

    return ret;
}

void ffParseMouseJsonObject(FFMouseOptions* options, yyjson_val* module)
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

        ffPrintError(FF_MOUSE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateMouseJsonConfig(FFMouseOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    if (options->ignores.length > 0)
    {
        yyjson_mut_val* ignores = yyjson_mut_obj_add_arr(doc, module, "ignores");
        FF_LIST_FOR_EACH(FFstrbuf, strbuf, options->ignores)
            yyjson_mut_arr_append(ignores, yyjson_mut_strncpy(doc, strbuf->chars, strbuf->length));
    }
}

bool ffGenerateMouseJsonResult(FF_MAYBE_UNUSED FFMouseOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFMouseDevice));

    const char* error = ffDetectMouse(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
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

    FF_LIST_FOR_EACH(FFMouseDevice, device, result)
    {
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->name);
    }

    return true;
}

void ffInitMouseOptions(FFMouseOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰍽");

    ffListInit(&options->ignores, sizeof(FFstrbuf));
}

void ffDestroyMouseOptions(FFMouseOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    FF_LIST_FOR_EACH(FFstrbuf, str, options->ignores)
        ffStrbufDestroy(str);
    ffListDestroy(&options->ignores);
}

FFModuleBaseInfo ffMouseModuleInfo = {
    .name = FF_MOUSE_MODULE_NAME,
    .description = "List connected mouses",
    .initOptions = (void*) ffInitMouseOptions,
    .destroyOptions = (void*) ffDestroyMouseOptions,
    .parseJsonObject = (void*) ffParseMouseJsonObject,
    .printModule = (void*) ffPrintMouse,
    .generateJsonResult = (void*) ffGenerateMouseJsonResult,
    .generateJsonConfig = (void*) ffGenerateMouseJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Mouse name", "name"},
        {"Mouse serial number", "serial"},
    }))
};
