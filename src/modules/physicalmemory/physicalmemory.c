#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/percent.h"
#include "detection/physicalmemory/physicalmemory.h"
#include "modules/physicalmemory/physicalmemory.h"
#include "util/stringUtils.h"

#define FF_PHYSICALMEMORY_DISPLAY_NAME "Physical Memory"

void ffPrintPhysicalMemory(FFPhysicalMemoryOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalMemoryResult));
    const char* error = ffDetectPhysicalMemory(&result);

    if(error)
    {
        ffPrintError(FF_PHYSICALMEMORY_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if (result.length == 0)
    {
        ffPrintError(FF_PHYSICALMEMORY_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No physical memory detected");
        return;
    }

    FF_STRBUF_AUTO_DESTROY prettySize = ffStrbufCreate();

    uint32_t i = 0;
    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        ++i;
        ffStrbufClear(&prettySize);
        ffParseSize(device->size, &prettySize);

        if (options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_PHYSICALMEMORY_DISPLAY_NAME, result.length == 1 ? 0 : (uint8_t) i, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            fputs(prettySize.chars, stdout);
            fputs(" - ", stdout);
            ffStrbufWriteTo(&device->type, stdout);
            if (device->maxSpeed > 0)
                printf("-%u", device->maxSpeed);
            if (device->runningSpeed > 0 && device->runningSpeed != device->maxSpeed)
                printf(" @ %u MT/s", device->runningSpeed);
            if (device->vendor.length > 0)
                printf(" (%s)", device->vendor.chars);
            if (device->ecc)
                fputs(" - ECC", stdout);
            putchar('\n');
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_PHYSICALMEMORY_DISPLAY_NAME, (uint8_t) i, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                FF_FORMAT_ARG(device->size, "bytes"),
                FF_FORMAT_ARG(prettySize, "size"),
                FF_FORMAT_ARG(device->maxSpeed, "max-speed"),
                FF_FORMAT_ARG(device->runningSpeed, "running-speed"),
                FF_FORMAT_ARG(device->type, "type"),
                FF_FORMAT_ARG(device->formFactor, "form-factor"),
                FF_FORMAT_ARG(device->locator, "locator"),
                FF_FORMAT_ARG(device->vendor, "vendor"),
                FF_FORMAT_ARG(device->serial, "serial"),
                FF_FORMAT_ARG(device->partNumber, "part-number"),
                FF_FORMAT_ARG(device->ecc, "is-ecc-enabled"),
            }));
        }
    }

    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->locator);
        ffStrbufDestroy(&device->formFactor);
        ffStrbufDestroy(&device->vendor);
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->partNumber);
    }
}

bool ffParsePhysicalMemoryCommandOptions(FFPhysicalMemoryOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PHYSICALMEMORY_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParsePhysicalMemoryJsonObject(FFPhysicalMemoryOptions* options, yyjson_val* module)
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

        ffPrintError(FF_PHYSICALMEMORY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGeneratePhysicalMemoryJsonConfig(FFPhysicalMemoryOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyPhysicalMemoryOptions))) FFPhysicalMemoryOptions defaultOptions;
    ffInitPhysicalMemoryOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGeneratePhysicalMemoryJsonResult(FF_MAYBE_UNUSED FFPhysicalMemoryOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalMemoryResult));
    const char* error = ffDetectPhysicalMemory(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_uint(doc, obj, "size", device->size);
        yyjson_mut_obj_add_uint(doc, obj, "maxSpeed", device->maxSpeed);
        yyjson_mut_obj_add_uint(doc, obj, "runningSpeed", device->runningSpeed);
        yyjson_mut_obj_add_strbuf(doc, obj, "type", &device->type);
        yyjson_mut_obj_add_strbuf(doc, obj, "locator", &device->locator);
        yyjson_mut_obj_add_strbuf(doc, obj, "formFactor", &device->formFactor);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &device->vendor);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &device->serial);
        yyjson_mut_obj_add_strbuf(doc, obj, "partNumber", &device->partNumber);
        yyjson_mut_obj_add_bool(doc, obj, "ecc", device->ecc);
    }

    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->locator);
        ffStrbufDestroy(&device->formFactor);
        ffStrbufDestroy(&device->vendor);
        ffStrbufDestroy(&device->serial);
        ffStrbufDestroy(&device->partNumber);
    }
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_PHYSICALMEMORY_MODULE_NAME,
    .description = "Print system physical memory devices",
    .parseCommandOptions = (void*) ffParsePhysicalMemoryCommandOptions,
    .parseJsonObject = (void*) ffParsePhysicalMemoryJsonObject,
    .printModule = (void*) ffPrintPhysicalMemory,
    .generateJsonConfig = (void*) ffGeneratePhysicalMemoryJsonConfig,
    .generateJsonResult = (void*) ffGeneratePhysicalMemoryJsonResult,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Size (in bytes)", "bytes"},
        {"Size formatted", "size"},
        {"Max speed (in MT/s)", "max-speed"},
        {"Running speed (in MT/s)", "running-speed"},
        {"Type (DDR4, DDR5, etc.)", "type"},
        {"Form factor (SODIMM, DIMM, etc.)", "form-factor"},
        {"Bank/Device Locator (BANK0/SIMM0, BANK0/SIMM1, etc.)", "locator"},
        {"Vendor", "vendor"},
        {"Serial number", "serial"},
        {"Part number", "part-number"},
        {"True if ECC enabled", "is-ecc-enabled"},
    }))
};

void ffInitPhysicalMemoryOptions(FFPhysicalMemoryOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "󰑭");
}

void ffDestroyPhysicalMemoryOptions(FFPhysicalMemoryOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
