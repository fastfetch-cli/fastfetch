#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "detection/diskio/diskio.h"
#include "modules/diskio/diskio.h"
#include "util/stringUtils.h"

#define FF_DISKIO_DISPLAY_NAME "Disk IO"
#define FF_DISKIO_NUM_FORMAT_ARGS 10

static int sortDevices(const FFDiskIOResult* left, const FFDiskIOResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFDiskIOOptions* options, FFDiskIOResult* dev, uint32_t index, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        if(!dev->name.length)
            ffStrbufSetF(&dev->name, "unknown %u", (unsigned) index);

        ffStrbufSetF(key, FF_DISKIO_DISPLAY_NAME " (%s)", dev->name.chars);
    }
    else
    {
        ffStrbufClear(key);
        ffParseFormatString(key, &options->moduleArgs.key, 2, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &index},
            {FF_FORMAT_ARG_TYPE_STRBUF, &dev->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &dev->devPath},
        });
    }
}

void ffPrintDiskIO(FFDiskIOOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFDiskIOResult));
    const char* error = ffDetectDiskIO(&result, options);

    if(error)
    {
        ffPrintError(FF_DISKIO_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    ffListSort(&result, (const void*) sortDevices);

    uint32_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer2 = ffStrbufCreate();

    FF_LIST_FOR_EACH(FFDiskIOResult, dev, result)
    {
        formatKey(options, dev, result.length == 1 ? 0 : index + 1, &key);
        ffStrbufClear(&buffer);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            ffParseSize(dev->bytesRead, &buffer);
            ffStrbufAppendS(&buffer, "/s (R) - ");
            ffParseSize(dev->bytesWritten, &buffer);
            ffStrbufAppendS(&buffer, "/s (W)");
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            ffStrbufClear(&buffer2);
            ffParseSize(dev->bytesRead, &buffer);
            ffStrbufAppendS(&buffer, "/s");
            ffParseSize(dev->bytesWritten, &buffer2);
            ffStrbufAppendS(&buffer2, "/s");

            const char* physicalType;
            switch(dev->type)
            {
                case FF_DISKIO_PHYSICAL_TYPE_HDD:
                    physicalType = "HDD";
                    break;
                case FF_DISKIO_PHYSICAL_TYPE_SSD:
                    physicalType = "SSD";
                    break;
                default:
                    physicalType = "Unknown";
                    break;
            }

            ffPrintFormatString(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_DISKIO_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &buffer},
                {FF_FORMAT_ARG_TYPE_STRBUF, &buffer2},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->interconnect},
                {FF_FORMAT_ARG_TYPE_STRING, physicalType},
                {FF_FORMAT_ARG_TYPE_STRBUF, &dev->devPath},
                {FF_FORMAT_ARG_TYPE_UINT64, &dev->bytesRead},
                {FF_FORMAT_ARG_TYPE_UINT64, &dev->bytesWritten},
                {FF_FORMAT_ARG_TYPE_UINT64, &dev->readCount},
                {FF_FORMAT_ARG_TYPE_UINT64, &dev->writeCount},
            });
        }
        ++index;
    }

    FF_LIST_FOR_EACH(FFDiskIOResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
    }
}

bool ffParseDiskIOCommandOptions(FFDiskIOOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DISKIO_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "name-prefix"))
    {
        ffOptionParseString(key, value, &options->namePrefix);
        return true;
    }

    return false;
}

void ffParseDiskIOJsonObject(FFDiskIOOptions* options, yyjson_val* module)
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

        ffPrintError(FF_DISKIO_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateDiskIOJsonConfig(FFDiskIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDiskIOOptions))) FFDiskIOOptions defaultOptions;
    ffInitDiskIOOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (!ffStrbufEqual(&options->namePrefix, &defaultOptions.namePrefix))
        yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);
}

void ffGenerateDiskIOJsonResult(FFDiskIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFDiskIOResult));
    const char* error = ffDetectDiskIO(&result, options);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFDiskIOResult, dev, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &dev->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "interconnectType", &dev->interconnect);
        yyjson_mut_obj_add_strbuf(doc, obj, "devPath", &dev->devPath);

        switch(dev->type)
        {
            case FF_DISKIO_PHYSICAL_TYPE_HDD:
                yyjson_mut_obj_add_str(doc, obj, "physicalType", "HDD");
                break;
            case FF_DISKIO_PHYSICAL_TYPE_SSD:
                yyjson_mut_obj_add_str(doc, obj, "physicalType", "SSD");
                break;
            default:
                yyjson_mut_obj_add_null(doc, obj, "physicalType");
                break;
        }

        yyjson_mut_obj_add_uint(doc, obj, "bytesRead", dev->bytesRead);
        yyjson_mut_obj_add_uint(doc, obj, "bytesWritten", dev->bytesWritten);
        yyjson_mut_obj_add_uint(doc, obj, "readCount", dev->readCount);
        yyjson_mut_obj_add_uint(doc, obj, "writeCount", dev->writeCount);
    }

    FF_LIST_FOR_EACH(FFDiskIOResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
    }
}

void ffPrintDiskIOHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_DISKIO_MODULE_NAME, "{1} (R) - {2} (W)", FF_DISKIO_NUM_FORMAT_ARGS, (const char* []) {
        "Size of data read per second (formatted)",
        "Size of data written per second (formatted)",
        "Device name",
        "Device interconnect type",
        "Device physical type (SSD / HDD)",
        "Device raw file path",
        "Size of data read per second (in bytes)",
        "Size of data written per second (in bytes)",
        "Number of reads",
        "Number of writes",
    });
}

void ffInitDiskIOOptions(FFDiskIOOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DISKIO_MODULE_NAME,
        ffParseDiskIOCommandOptions,
        ffParseDiskIOJsonObject,
        ffPrintDiskIO,
        ffGenerateDiskIOJsonResult,
        ffPrintDiskIOHelpFormat,
        ffGenerateDiskIOJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInit(&options->namePrefix);
}

void ffDestroyDiskIOOptions(FFDiskIOOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}
