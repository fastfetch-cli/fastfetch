#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "detection/diskio/diskio.h"
#include "modules/diskio/diskio.h"
#include "util/stringUtils.h"

#define FF_DISKIO_DISPLAY_NAME "Disk IO"
#define FF_DISKIO_NUM_FORMAT_ARGS 8

static int sortDevices(const FFDiskIOResult* left, const FFDiskIOResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFDiskIOOptions* options, FFDiskIOResult* dev, uint32_t index, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        ffStrbufSetF(key, FF_DISKIO_DISPLAY_NAME " (%s)", dev->name.length ? dev->name.chars : dev->devPath.chars);
    }
    else
    {
        ffStrbufClear(key);
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, 4, ((FFformatarg[]){
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(dev->name, "name"),
            FF_FORMAT_ARG(dev->devPath, "dev-path"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }
}

void ffPrintDiskIO(FFDiskIOOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFDiskIOResult));
    const char* error = ffDetectDiskIO(&result, options);

    if(error)
    {
        ffPrintError(FF_DISKIO_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, "%s", error);
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
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            ffParseSize(dev->bytesRead, &buffer);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");
            ffStrbufAppendS(&buffer, " (R) - ");

            ffParseSize(dev->bytesWritten, &buffer);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");
            ffStrbufAppendS(&buffer, " (W)");
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            ffStrbufClear(&buffer2);
            ffParseSize(dev->bytesRead, &buffer);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");
            ffParseSize(dev->bytesWritten, &buffer2);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");

            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_DISKIO_NUM_FORMAT_ARGS, ((FFformatarg[]){
                FF_FORMAT_ARG(buffer, "size-read"),
                FF_FORMAT_ARG(buffer2, "size-written"),
                FF_FORMAT_ARG(dev->name, "name"),
                FF_FORMAT_ARG(dev->devPath, "dev-path"),
                FF_FORMAT_ARG(dev->bytesRead, "bytes-read"),
                FF_FORMAT_ARG(dev->bytesWritten, "bytes-written"),
                FF_FORMAT_ARG(dev->readCount, "read-count"),
                FF_FORMAT_ARG(dev->writeCount, "write-count"),
            }));
        }
        ++index;
    }

    FF_LIST_FOR_EACH(FFDiskIOResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
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

    if (ffStrEqualsIgnCase(subKey, "detect-total"))
    {
        options->detectTotal = ffOptionParseBoolean(value);
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

        if (ffStrEqualsIgnCase(key, "detectTotal"))
        {
            options->detectTotal = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_DISKIO_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateDiskIOJsonConfig(FFDiskIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDiskIOOptions))) FFDiskIOOptions defaultOptions;
    ffInitDiskIOOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (!ffStrbufEqual(&options->namePrefix, &defaultOptions.namePrefix))
        yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);

    if (defaultOptions.detectTotal != options->detectTotal)
        yyjson_mut_obj_add_bool(doc, module, "detectTotal", options->detectTotal);
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
        yyjson_mut_obj_add_strbuf(doc, obj, "devPath", &dev->devPath);
        yyjson_mut_obj_add_uint(doc, obj, "bytesRead", dev->bytesRead);
        yyjson_mut_obj_add_uint(doc, obj, "bytesWritten", dev->bytesWritten);
        yyjson_mut_obj_add_uint(doc, obj, "readCount", dev->readCount);
        yyjson_mut_obj_add_uint(doc, obj, "writeCount", dev->writeCount);
    }

    FF_LIST_FOR_EACH(FFDiskIOResult, dev, result)
    {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->devPath);
    }
}

void ffPrintDiskIOHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_DISKIO_MODULE_NAME, "{1} (R) - {2} (W)", FF_DISKIO_NUM_FORMAT_ARGS, ((const char* []) {
        "Size of data read [per second] (formatted) - size-read",
        "Size of data written [per second] (formatted) - size-written",
        "Device name - name",
        "Device raw file path - dev-path",
        "Size of data read [per second] (in bytes) - bytes-read",
        "Size of data written [per second] (in bytes) - bytes-written",
        "Number of reads - read-count",
        "Number of writes - write-count",
    }));
}

void ffInitDiskIOOptions(FFDiskIOOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DISKIO_MODULE_NAME,
        "Print physical disk I/O throughput",
        ffParseDiskIOCommandOptions,
        ffParseDiskIOJsonObject,
        ffPrintDiskIO,
        ffGenerateDiskIOJsonResult,
        ffPrintDiskIOHelpFormat,
        ffGenerateDiskIOJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰓅");

    ffStrbufInit(&options->namePrefix);
    options->detectTotal = false;
}

void ffDestroyDiskIOOptions(FFDiskIOOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}
