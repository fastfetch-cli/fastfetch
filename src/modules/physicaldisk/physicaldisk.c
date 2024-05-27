#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/temps.h"
#include "detection/physicaldisk/physicaldisk.h"
#include "modules/physicaldisk/physicaldisk.h"
#include "util/stringUtils.h"

#define FF_PHYSICALDISK_DISPLAY_NAME "Physical Disk"
#define FF_PHYSICALDISK_NUM_FORMAT_ARGS 10

static int sortDevices(const FFPhysicalDiskResult* left, const FFPhysicalDiskResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFPhysicalDiskOptions* options, FFPhysicalDiskResult* dev, uint32_t index, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        ffStrbufSetF(key, FF_PHYSICALDISK_DISPLAY_NAME " (%s)", dev->name.length ? dev->name.chars : dev->devPath.chars);
    }
    else
    {
        ffStrbufClear(key);
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, 3, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &index, "index"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &dev->name, "name"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &dev->devPath, "dev-path"},
        }));
    }
}

void ffPrintPhysicalDisk(FFPhysicalDiskOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalDiskResult));
    const char* error = ffDetectPhysicalDisk(&result, options);

    if(error)
    {
        ffPrintError(FF_PHYSICALDISK_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    ffListSort(&result, (const void*) sortDevices);

    uint32_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    FF_LIST_FOR_EACH(FFPhysicalDiskResult, dev, result)
    {
        formatKey(options, dev, result.length == 1 ? 0 : index + 1, &key);
        ffStrbufClear(&buffer);
        ffParseSize(dev->size, &buffer);

        const char* physicalType = dev->type & FF_PHYSICALDISK_TYPE_HDD
            ? "HDD"
            : dev->type & FF_PHYSICALDISK_TYPE_SSD
                ? "SSD"
                : "";
        const char* removableType = dev->type & FF_PHYSICALDISK_TYPE_REMOVABLE
            ? "Removable"
            : dev->type & FF_PHYSICALDISK_TYPE_FIXED
                ? "Fixed"
                : "";
        const char* readOnlyType = dev->type & FF_PHYSICALDISK_TYPE_READONLY
            ? "Read-only"
            : "";

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            if (physicalType[0] || removableType[0] || readOnlyType[0])
            {
                ffStrbufAppendS(&buffer, " [");
                if (physicalType[0])
                    ffStrbufAppendS(&buffer, physicalType);
                if (removableType[0])
                {
                    if (buffer.chars[buffer.length - 1] != '[')
                        ffStrbufAppendS(&buffer, ", ");
                    ffStrbufAppendS(&buffer, removableType);
                }
                if (readOnlyType[0])
                {
                    if (buffer.chars[buffer.length - 1] != '[')
                        ffStrbufAppendS(&buffer, ", ");
                    ffStrbufAppendS(&buffer, readOnlyType);
                }
                ffStrbufAppendC(&buffer, ']');
            }

            if (dev->temperature == dev->temperature) //FF_PHYSICALDISK_TEMP_UNSET
            {
                if(buffer.length > 0)
                    ffStrbufAppendS(&buffer, " - ");

                ffTempsAppendNum(dev->temperature, &buffer, options->tempConfig, &options->moduleArgs);
            }
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            FF_STRBUF_AUTO_DESTROY tempStr = ffStrbufCreate();
            ffTempsAppendNum(dev->temperature, &tempStr, options->tempConfig, &options->moduleArgs);
            if (dev->type & FF_PHYSICALDISK_TYPE_READWRITE)
                readOnlyType = "Read-write";
            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_PHYSICALDISK_NUM_FORMAT_ARGS, ((FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &buffer, "size"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->name, "name"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->interconnect, "interconnect"},
                {FF_FORMAT_ARG_TYPE_STRING, physicalType, "type"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->devPath, "dev-path"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->serial, "serial"},
                {FF_FORMAT_ARG_TYPE_STRING, removableType, "removable-type"},
                {FF_FORMAT_ARG_TYPE_STRING, readOnlyType, "readonly-type"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->revision, "revision"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &tempStr, "temperature"},
            }));
        }
        ++index;
    }

    FF_LIST_FOR_EACH(FFPhysicalDiskResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
        ffStrbufDestroy(&dev->serial);
        ffStrbufDestroy(&dev->revision);
    }
}

bool ffParsePhysicalDiskCommandOptions(FFPhysicalDiskOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PHYSICALDISK_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "name-prefix"))
    {
        ffOptionParseString(key, value, &options->namePrefix);
        return true;
    }

    if (ffTempsParseCommandOptions(key, subKey, value, &options->temp, &options->tempConfig))
        return true;

    return false;
}

void ffParsePhysicalDiskJsonObject(FFPhysicalDiskOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "namePrefix"))
        {
            ffStrbufSetS(&options->namePrefix, yyjson_get_str(val));
            continue;
        }

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig))
            continue;

        ffPrintError(FF_PHYSICALDISK_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGeneratePhysicalDiskJsonConfig(FFPhysicalDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyPhysicalDiskOptions))) FFPhysicalDiskOptions defaultOptions;
    ffInitPhysicalDiskOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (!ffStrbufEqual(&options->namePrefix, &defaultOptions.namePrefix))
        yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);

    ffTempsGenerateJsonConfig(doc, module, defaultOptions.temp, defaultOptions.tempConfig, options->temp, options->tempConfig);
}

void ffGeneratePhysicalDiskJsonResult(FFPhysicalDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalDiskResult));
    const char* error = ffDetectPhysicalDisk(&result, options);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFPhysicalDiskResult, dev, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &dev->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "devPath", &dev->devPath);
        yyjson_mut_obj_add_strbuf(doc, obj, "interconnect", &dev->interconnect);

        if (dev->type & FF_PHYSICALDISK_TYPE_HDD)
            yyjson_mut_obj_add_str(doc, obj, "kind", "HDD");
        else if (dev->type & FF_PHYSICALDISK_TYPE_SSD)
            yyjson_mut_obj_add_str(doc, obj, "kind", "SSD");
        else
            yyjson_mut_obj_add_null(doc, obj, "kind");

        yyjson_mut_obj_add_uint(doc, obj, "size", dev->size);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &dev->serial);

        if (dev->type & FF_PHYSICALDISK_TYPE_REMOVABLE)
            yyjson_mut_obj_add_bool(doc, obj, "removable", true);
        else if (dev->type & FF_PHYSICALDISK_TYPE_FIXED)
            yyjson_mut_obj_add_bool(doc, obj, "removable", false);
        else
            yyjson_mut_obj_add_null(doc, obj, "removable");

        if (dev->type & FF_PHYSICALDISK_TYPE_READONLY)
            yyjson_mut_obj_add_bool(doc, obj, "readOnly", true);
        else if (dev->type & FF_PHYSICALDISK_TYPE_READWRITE)
            yyjson_mut_obj_add_bool(doc, obj, "readOnly", false);
        else
            yyjson_mut_obj_add_null(doc, obj, "readOnly");

        yyjson_mut_obj_add_strbuf(doc, obj, "revision", &dev->revision);

        yyjson_mut_obj_add_real(doc, obj, "temperature", dev->temperature);
    }

    FF_LIST_FOR_EACH(FFPhysicalDiskResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
        ffStrbufDestroy(&dev->serial);
        ffStrbufDestroy(&dev->revision);
    }
}

void ffPrintPhysicalDiskHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_PHYSICALDISK_MODULE_NAME, "{1} [{6}, {7}, {8}]", FF_PHYSICALDISK_NUM_FORMAT_ARGS, ((const char* []) {
        "Device size (formatted) - ",
        "Device name",
        "Device interconnect type",
        "Device raw file path",
        "Serial number",
        "Device kind (SSD or HDD)",
        "Device kind (Removable or Fixed)",
        "Device kind (Read-only or Read-write)",
        "Product revision",
        "Device temperature (formatted)",
    }));
}

void ffInitPhysicalDiskOptions(FFPhysicalDiskOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_PHYSICALDISK_MODULE_NAME,
        "Print physical disk information",
        ffParsePhysicalDiskCommandOptions,
        ffParsePhysicalDiskJsonObject,
        ffPrintPhysicalDisk,
        ffGeneratePhysicalDiskJsonResult,
        ffPrintPhysicalDiskHelpFormat,
        ffGeneratePhysicalDiskJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInit(&options->namePrefix);
    options->temp = false;
    options->tempConfig = (FFColorRangeConfig) { 40, 60 };
}

void ffDestroyPhysicalDiskOptions(FFPhysicalDiskOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}
