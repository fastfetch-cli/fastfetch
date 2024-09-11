#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/libc/libc.h"
#include "detection/camera/camera.h"
#include "modules/camera/camera.h"
#include "util/stringUtils.h"

#define FF_CAMERA_NUM_FORMAT_ARGS 6

static void printDevice(FFCameraOptions* options, const FFCameraResult* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CAMERA_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufWriteTo(&device->name, stdout);
        if (device->colorspace.length > 0)
        {
            fputs(" - ", stdout);
            ffStrbufWriteTo(&device->colorspace, stdout);
        }

        if (device->width > 0 && device->height > 0)
            printf(" (%ux%u px)\n", (unsigned) device->width, (unsigned) device->height);
        else
            putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_CAMERA_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_CAMERA_NUM_FORMAT_ARGS, (((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->vendor, "vendor"),
            FF_FORMAT_ARG(device->colorspace, "colorspace"),
            FF_FORMAT_ARG(device->id, "id"),
            FF_FORMAT_ARG(device->width, "width"),
            FF_FORMAT_ARG(device->height, "height"),
        })));
    }
}

void ffPrintCamera(FFCameraOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFCameraResult));
    const char* error = ffDetectCamera(&result);

    if (error)
    {
        ffPrintError(FF_CAMERA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if (result.length == 0)
    {
        ffPrintError(FF_CAMERA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No camera found");
        return;
    }

    for(uint32_t i = 0; i < result.length; i++)
    {
        uint8_t index = (uint8_t) (result.length == 1 ? 0 : i + 1);
        printDevice(options, FF_LIST_GET(FFCameraResult, result, i), index);
    }

    FF_LIST_FOR_EACH(FFCameraResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->id);
        ffStrbufDestroy(&dev->colorspace);
    }
}

bool ffParseCameraCommandOptions(FFCameraOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CAMERA_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseCameraJsonObject(FFCameraOptions* options, yyjson_val* module)
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

        ffPrintError(FF_CAMERA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateCameraJsonConfig(FFCameraOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCameraOptions))) FFCameraOptions defaultOptions;
    ffInitCameraOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateCameraJsonResult(FF_MAYBE_UNUSED FFCameraOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFCameraResult));
    const char* error = ffDetectCamera(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFCameraResult, dev, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &dev->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &dev->vendor);
        yyjson_mut_obj_add_strbuf(doc, obj, "colorSpace", &dev->colorspace);
        yyjson_mut_obj_add_strbuf(doc, obj, "id", &dev->id);
        yyjson_mut_obj_add_uint(doc, obj, "width", dev->width);
        yyjson_mut_obj_add_uint(doc, obj, "height", dev->height);
    }

    FF_LIST_FOR_EACH(FFCameraResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->id);
        ffStrbufDestroy(&dev->colorspace);
    }
}

void ffPrintCameraHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_CAMERA_MODULE_NAME, "{1} ({4}px x {5}px)", FF_CAMERA_NUM_FORMAT_ARGS, ((const char* []) {
        "Device name - name",
        "Vendor - vendor",
        "Color space - colorspace",
        "Identifier - id",
        "Width (in px) - width",
        "Height (in px) - height",
    }));
}

void ffInitCameraOptions(FFCameraOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CAMERA_MODULE_NAME,
        "Print available cameras",
        ffParseCameraCommandOptions,
        ffParseCameraJsonObject,
        ffPrintCamera,
        ffGenerateCameraJsonResult,
        ffPrintCameraHelpFormat,
        ffGenerateCameraJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰄀");
}

void ffDestroyCameraOptions(FFCameraOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
