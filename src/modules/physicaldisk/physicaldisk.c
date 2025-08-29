#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/temps.h"
#include "common/size.h"
#include "detection/physicaldisk/physicaldisk.h"
#include "modules/physicaldisk/physicaldisk.h"
#include "util/stringUtils.h"

#define FF_PHYSICALDISK_DISPLAY_NAME "Physical Disk"

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
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, ((FFformatarg[]){
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(dev->name, "name"),
            FF_FORMAT_ARG(dev->devPath, "dev-path"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }
}

bool ffPrintPhysicalDisk(FFPhysicalDiskOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalDiskResult));
    const char* error = ffDetectPhysicalDisk(&result, options);

    if(error)
    {
        ffPrintError(FF_PHYSICALDISK_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    ffListSort(&result, (const void*) sortDevices);

    uint32_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    FF_LIST_FOR_EACH(FFPhysicalDiskResult, dev, result)
    {
        formatKey(options, dev, result.length == 1 ? 0 : index + 1, &key);
        ffStrbufClear(&buffer);
        ffSizeAppendNum(dev->size, &buffer);

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
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

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

            if (dev->temperature != FF_PHYSICALDISK_TEMP_UNSET)
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
            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
                FF_FORMAT_ARG(buffer, "size"),
                FF_FORMAT_ARG(dev->name, "name"),
                FF_FORMAT_ARG(dev->interconnect, "interconnect"),
                FF_FORMAT_ARG(dev->devPath, "dev-path"),
                FF_FORMAT_ARG(dev->serial, "serial"),
                FF_FORMAT_ARG(physicalType, "physical-type"),
                FF_FORMAT_ARG(removableType, "removable-type"),
                FF_FORMAT_ARG(readOnlyType, "readonly-type"),
                FF_FORMAT_ARG(dev->revision, "revision"),
                FF_FORMAT_ARG(tempStr, "temperature"),
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

    return true;
}

void ffParsePhysicalDiskJsonObject(FFPhysicalDiskOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "namePrefix"))
        {
            ffStrbufSetJsonVal(&options->namePrefix, val);
            continue;
        }

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig))
            continue;

        ffPrintError(FF_PHYSICALDISK_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGeneratePhysicalDiskJsonConfig(FFPhysicalDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);

    ffTempsGenerateJsonConfig(doc, module, options->temp, options->tempConfig);
}

bool ffGeneratePhysicalDiskJsonResult(FFPhysicalDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalDiskResult));
    const char* error = ffDetectPhysicalDisk(&result, options);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
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

        if (dev->temperature != FF_PHYSICALDISK_TEMP_UNSET)
            yyjson_mut_obj_add_real(doc, obj, "temperature", dev->temperature);
        else
            yyjson_mut_obj_add_null(doc, obj, "temperature");
    }

    FF_LIST_FOR_EACH(FFPhysicalDiskResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
        ffStrbufDestroy(&dev->serial);
        ffStrbufDestroy(&dev->revision);
    }

    return true;
}

void ffInitPhysicalDiskOptions(FFPhysicalDiskOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰋊");

    ffStrbufInit(&options->namePrefix);
    options->temp = false;
    options->tempConfig = (FFColorRangeConfig) { 50, 70 };
}

void ffDestroyPhysicalDiskOptions(FFPhysicalDiskOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}

FFModuleBaseInfo ffPhysicalDiskModuleInfo = {
    .name = FF_PHYSICALDISK_MODULE_NAME,
    .description = "Print physical disk information",
    .initOptions = (void*) ffInitPhysicalDiskOptions,
    .destroyOptions = (void*) ffDestroyPhysicalDiskOptions,
    .parseJsonObject = (void*) ffParsePhysicalDiskJsonObject,
    .printModule = (void*) ffPrintPhysicalDisk,
    .generateJsonResult = (void*) ffGeneratePhysicalDiskJsonResult,
    .generateJsonConfig = (void*) ffGeneratePhysicalDiskJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Device size (formatted)", "size"},
        {"Device name", "name"},
        {"Device interconnect type", "interconnect"},
        {"Device raw file path", "dev-path"},
        {"Serial number", "serial"},
        {"Device kind (SSD or HDD)", "physical-type"},
        {"Device kind (Removable or Fixed)", "removable-type"},
        {"Device kind (Read-only or Read-write)", "readonly-type"},
        {"Product revision", "revision"},
        {"Device temperature (formatted)", "temperature"},
    }))
};
