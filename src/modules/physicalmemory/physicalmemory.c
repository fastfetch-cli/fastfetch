#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/percent.h"
#include "detection/physicalmemory/physicalmemory.h"
#include "modules/physicalmemory/physicalmemory.h"
#include "util/stringUtils.h"

#define FF_PHYSICALMEMORY_NUM_FORMAT_ARGS 9
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

    FF_STRBUF_AUTO_DESTROY prettySize = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    uint32_t i = 0;
    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        ++i;
        ffStrbufClear(&prettySize);
        ffParseSize(device->size, &prettySize);

        if(options->moduleArgs.key.length == 0)
        {
            if (device->maxSpeed)
                ffStrbufSetF(&key, "%s (%s %s-%u)", FF_PHYSICALMEMORY_DISPLAY_NAME, device->vendor.chars, device->type.chars, device->maxSpeed);
            else
                ffStrbufSetF(&key, "%s (%s %s)", FF_PHYSICALMEMORY_DISPLAY_NAME, device->vendor.chars, device->type.chars);
        }
        else
        {
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 5, ((FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_UINT, &i},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->vendor},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->type},
                {FF_FORMAT_ARG_TYPE_UINT, &device->maxSpeed},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->deviceLocator},
            }));
        }

        if (options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            if (device->runningSpeed > 0 && device->runningSpeed != device->maxSpeed)
                printf("%s, running at %u MT/s\n", prettySize.chars, device->runningSpeed);
            else
                puts(prettySize.chars);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_PHYSICALMEMORY_NUM_FORMAT_ARGS, ((FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_UINT64, &device->size},
                {FF_FORMAT_ARG_TYPE_STRBUF, &prettySize},
                {FF_FORMAT_ARG_TYPE_UINT, &device->maxSpeed},
                {FF_FORMAT_ARG_TYPE_UINT, &device->runningSpeed},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->type},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->formFactor},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &device->deviceLocator},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->vendor},
                {FF_FORMAT_ARG_TYPE_STRBUF, &device->serial},
            }));
        }
    }

    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->formFactor);
        ffStrbufDestroy(&device->vendor);
        ffStrbufDestroy(&device->serial);
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
        yyjson_mut_obj_add_strbuf(doc, obj, "formFactor", &device->formFactor);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &device->vendor);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &device->serial);
    }

    FF_LIST_FOR_EACH(FFPhysicalMemoryResult, device, result)
    {
        ffStrbufDestroy(&device->type);
        ffStrbufDestroy(&device->formFactor);
        ffStrbufDestroy(&device->vendor);
        ffStrbufDestroy(&device->serial);
    }
}

void ffPrintPhysicalMemoryHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_PHYSICALMEMORY_MODULE_NAME, "{7} {5}-{3}: {2}, running at {4} MT/s", FF_PHYSICALMEMORY_NUM_FORMAT_ARGS, ((const char* []) {
        "Size (in bytes)",
        "Size formatted",
        "Max speed (in MT/s)",
        "Running speed (in MT/s)",
        "Type (DDR4, DDR5, etc.)",
        "Form factor (SODIMM, DIMM, etc.)",
        "Device locator (SIMM1, SIMM2, etc.)",
        "Vendor",
        "Serial number",
    }));
}

void ffInitPhysicalMemoryOptions(FFPhysicalMemoryOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_PHYSICALMEMORY_MODULE_NAME,
        "Print system physical memory devices",
        ffParsePhysicalMemoryCommandOptions,
        ffParsePhysicalMemoryJsonObject,
        ffPrintPhysicalMemory,
        ffGeneratePhysicalMemoryJsonResult,
        ffPrintPhysicalMemoryHelpFormat,
        ffGeneratePhysicalMemoryJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyPhysicalMemoryOptions(FFPhysicalMemoryOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
