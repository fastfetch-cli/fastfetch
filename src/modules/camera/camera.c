#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/libc/libc.h"
#include "detection/camera/camera.h"
#include "modules/camera/camera.h"
#include "util/stringUtils.h"

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
        FF_PRINT_FORMAT_CHECKED(FF_CAMERA_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, (((FFformatarg[]) {
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(device->vendor, "vendor"),
            FF_FORMAT_ARG(device->colorspace, "colorspace"),
            FF_FORMAT_ARG(device->id, "id"),
            FF_FORMAT_ARG(device->width, "width"),
            FF_FORMAT_ARG(device->height, "height"),
        })));
    }
}

bool ffPrintCamera(FFCameraOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFCameraResult));
    const char* error = ffDetectCamera(&result);

    if (error)
    {
        ffPrintError(FF_CAMERA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (result.length == 0)
    {
        ffPrintError(FF_CAMERA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No camera found");
        return false;
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

    return true;
}

void ffParseCameraJsonObject(FFCameraOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_CAMERA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCameraJsonConfig(FFCameraOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateCameraJsonResult(FF_MAYBE_UNUSED FFCameraOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFCameraResult));
    const char* error = ffDetectCamera(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
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

    return true;
}

void ffInitCameraOptions(FFCameraOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰄀");
}

void ffDestroyCameraOptions(FFCameraOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCameraModuleInfo = {
    .name = FF_CAMERA_MODULE_NAME,
    .description = "Print available cameras",
    .initOptions = (void*) ffInitCameraOptions,
    .destroyOptions = (void*) ffDestroyCameraOptions,
    .parseJsonObject = (void*) ffParseCameraJsonObject,
    .printModule = (void*) ffPrintCamera,
    .generateJsonResult = (void*) ffGenerateCameraJsonResult,
    .generateJsonConfig = (void*) ffGenerateCameraJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Device name", "name"},
        {"Vendor", "vendor"},
        {"Color space", "colorspace"},
        {"Identifier", "id"},
        {"Width (in px)", "width"},
        {"Height (in px)", "height"},
    }))
};
