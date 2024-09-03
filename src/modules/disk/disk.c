#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/percent.h"
#include "common/time.h"
#include "detection/disk/disk.h"
#include "modules/disk/disk.h"
#include "util/stringUtils.h"

#define FF_DISK_NUM_FORMAT_ARGS 14
#pragma GCC diagnostic ignored "-Wsign-conversion"

static void printDisk(FFDiskOptions* options, const FFDisk* disk)
{
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if(options->moduleArgs.key.length == 0)
    {
        if(instance.config.display.pipe)
            ffStrbufAppendF(&key, "%s (%s)", FF_DISK_MODULE_NAME, disk->mountpoint.chars);
        else
        {
            #ifdef __linux__
            if (getenv("WSL_DISTRO_NAME") != NULL && getenv("WT_SESSION") != NULL)
            {
                if (ffStrbufEqualS(&disk->filesystem, "9p") && ffStrbufStartsWithS(&disk->mountpoint, "/mnt/"))
                    ffStrbufAppendF(&key, "%s (\e]8;;file:///%c:/\e\\%s\e]8;;\e\\)", FF_DISK_MODULE_NAME, disk->mountpoint.chars[5], disk->mountpoint.chars);
                else
                    ffStrbufAppendF(&key, "%s (\e]8;;file:////wsl.localhost/%s%s\e\\%s\e]8;;\e\\)", FF_DISK_MODULE_NAME, getenv("WSL_DISTRO_NAME"), disk->mountpoint.chars, disk->mountpoint.chars);
            }
            else
            #endif
            ffStrbufAppendF(&key, "%s (\e]8;;file://%s\e\\%s\e]8;;\e\\)", FF_DISK_MODULE_NAME, disk->mountpoint.chars, disk->mountpoint.chars);
        }
    }
    else
    {
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 4, ((FFformatarg[]){
            FF_FORMAT_ARG(disk->mountpoint, "mountpoint"),
            FF_FORMAT_ARG(disk->name, "name"),
            FF_FORMAT_ARG(disk->mountFrom, "mount-from"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffParseSize(disk->bytesUsed, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffParseSize(disk->bytesTotal, &totalPretty);

    double bytesPercentage = disk->bytesTotal > 0 ? (double) disk->bytesUsed / (double) disk->bytesTotal * 100.0 : 0;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

        if(disk->bytesTotal > 0)
        {
            if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffPercentAppendBar(&str, bytesPercentage, options->percent, &options->moduleArgs);
                ffStrbufAppendC(&str, ' ');
            }

            if(!(instance.config.display.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendF(&str, "%s / %s ", usedPretty.chars, totalPretty.chars);

            if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                ffPercentAppendNum(&str, bytesPercentage, options->percent, str.length > 0, &options->moduleArgs);
                ffStrbufAppendC(&str, ' ');
            }
        }
        else
            ffStrbufAppendS(&str, "Unknown ");

        if(!(instance.config.display.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
        {
            if(disk->filesystem.length)
                ffStrbufAppendF(&str, "- %s ", disk->filesystem.chars);

            ffStrbufAppendC(&str, '[');
            if(disk->type & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT)
                ffStrbufAppendS(&str, "External, ");
            if(disk->type & FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT)
                ffStrbufAppendS(&str, "Subvolume, ");
            if(disk->type & FF_DISK_VOLUME_TYPE_HIDDEN_BIT)
                ffStrbufAppendS(&str, "Hidden, ");
            if(disk->type & FF_DISK_VOLUME_TYPE_READONLY_BIT)
                ffStrbufAppendS(&str, "Read-only, ");
            if (str.chars[str.length - 1] == '[')
                ffStrbufSubstrBefore(&str, str.length - 1);
            else
            {
                ffStrbufTrimRight(&str, ' ');
                str.chars[str.length - 1] = ']';
            }
        }

        ffStrbufTrimRight(&str, ' ');
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY bytesPercentageNum = ffStrbufCreate();
        ffPercentAppendNum(&bytesPercentageNum, bytesPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY bytesPercentageBar = ffStrbufCreate();
        ffPercentAppendBar(&bytesPercentageBar, bytesPercentage, options->percent, &options->moduleArgs);

        double filesPercentage = disk->filesTotal > 0 ? ((double) disk->filesUsed / (double) disk->filesTotal) * 100.0 : 0;
        FF_STRBUF_AUTO_DESTROY filesPercentageNum = ffStrbufCreate();
        ffPercentAppendNum(&filesPercentageNum, filesPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY filesPercentageBar = ffStrbufCreate();
        ffPercentAppendBar(&filesPercentageBar, filesPercentage, options->percent, &options->moduleArgs);

        bool isExternal = !!(disk->type & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT);
        bool isHidden = !!(disk->type & FF_DISK_VOLUME_TYPE_HIDDEN_BIT);
        bool isReadOnly = !!(disk->type & FF_DISK_VOLUME_TYPE_READONLY_BIT);

        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_DISK_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(usedPretty, "size-used"),
            FF_FORMAT_ARG(totalPretty, "size-total"),
            FF_FORMAT_ARG(bytesPercentageNum, "size-percentage"),
            FF_FORMAT_ARG(disk->filesUsed, "files-used"),
            FF_FORMAT_ARG(disk->filesTotal, "files-total"),
            FF_FORMAT_ARG(filesPercentageNum, "files-percentage"),
            FF_FORMAT_ARG(isExternal, "is-external"),
            FF_FORMAT_ARG(isHidden, "is-hidden"),
            FF_FORMAT_ARG(disk->filesystem, "filesystem"),
            FF_FORMAT_ARG(disk->name, "name"),
            FF_FORMAT_ARG(isReadOnly, "is-readonly"),
            {FF_FORMAT_ARG_TYPE_STRING, ffTimeToShortStr(disk->createTime), "create-time"},
            FF_FORMAT_ARG(bytesPercentageBar, "size-percentage-bar"),
            FF_FORMAT_ARG(filesPercentageBar, "files-percentage-bar"),
        }));
    }
}

void ffPrintDisk(FFDiskOptions* options)
{
    FF_LIST_AUTO_DESTROY disks = ffListCreate(sizeof (FFDisk));
    const char* error = ffDetectDisks(options, &disks);

    if(error)
    {
        ffPrintError(FF_DISK_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    }
    else
    {
        FF_LIST_FOR_EACH(FFDisk, disk, disks)
        {
            if(__builtin_expect(options->folders.length == 0, 1) && (disk->type & ~options->showTypes))
                continue;

            printDisk(options, disk);
        }
    }

    FF_LIST_FOR_EACH(FFDisk, disk, disks)
    {
        ffStrbufDestroy(&disk->mountFrom);
        ffStrbufDestroy(&disk->mountpoint);
        ffStrbufDestroy(&disk->filesystem);
        ffStrbufDestroy(&disk->name);
    }
}

bool ffParseDiskCommandOptions(FFDiskOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DISK_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "folders"))
    {
        ffOptionParseString(key, value, &options->folders);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-regular"))
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_VOLUME_TYPE_REGULAR_BIT;
        else
            options->showTypes &= ~FF_DISK_VOLUME_TYPE_REGULAR_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-external"))
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
        else
            options->showTypes &= ~FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-hidden"))
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
        else
            options->showTypes &= ~FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-subvolumes"))
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
        else
            options->showTypes &= ~FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-readonly"))
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
        else
            options->showTypes &= ~FF_DISK_VOLUME_TYPE_READONLY_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-unknown"))
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
        else
            options->showTypes &= ~FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "use-available"))
    {
        if (ffOptionParseBoolean(value))
            options->calcType = FF_DISK_CALC_TYPE_AVAILABLE;
        else
            options->calcType = FF_DISK_CALC_TYPE_FREE;
        return true;
    }

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseDiskJsonObject(FFDiskOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "folders"))
        {
            ffStrbufSetS(&options->folders, yyjson_get_str(val));
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showExternal"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showHidden"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showSubvolumes"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showReadOnly"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_READONLY_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showUnknown"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "useAvailable"))
        {
            if (yyjson_get_bool(val))
                options->calcType = FF_DISK_CALC_TYPE_AVAILABLE;
            else
                options->calcType = FF_DISK_CALC_TYPE_FREE;
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_DISK_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateDiskJsonConfig(FFDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDiskOptions))) FFDiskOptions defaultOptions;
    ffInitDiskOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.showTypes != options->showTypes)
    {
        if (options->showTypes & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showExternal", true);

        if (options->showTypes & FF_DISK_VOLUME_TYPE_HIDDEN_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showHidden", true);

        if (options->showTypes & FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showSubvolumes", true);

        if (options->showTypes & FF_DISK_VOLUME_TYPE_READONLY_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showReadOnly", true);

        if (options->showTypes & FF_DISK_VOLUME_TYPE_UNKNOWN_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showUnknown", true);
    }

    if (!ffStrbufEqual(&options->folders, &defaultOptions.folders))
        yyjson_mut_obj_add_strbuf(doc, module, "folders", &options->folders);

    if (defaultOptions.calcType != options->calcType)
        yyjson_mut_obj_add_bool(doc, module, "useAvailable", options->calcType == FF_DISK_CALC_TYPE_AVAILABLE);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateDiskJsonResult(FFDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY disks = ffListCreate(sizeof (FFDisk));
    const char* error = ffDetectDisks(options, &disks);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "result", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFDisk, item, disks)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);

        yyjson_mut_val* bytes = yyjson_mut_obj_add_obj(doc, obj, "bytes");
        yyjson_mut_obj_add_uint(doc, bytes, "available", item->bytesAvailable);
        yyjson_mut_obj_add_uint(doc, bytes, "free", item->bytesFree);
        yyjson_mut_obj_add_uint(doc, bytes, "total", item->bytesTotal);
        yyjson_mut_obj_add_uint(doc, bytes, "used", item->bytesUsed);

        yyjson_mut_val* files = yyjson_mut_obj_add_obj(doc, obj, "files");
        if (item->filesTotal == 0 && item->filesUsed == 0)
        {
            yyjson_mut_obj_add_null(doc, files, "total");
            yyjson_mut_obj_add_null(doc, files, "used");
        }
        else
        {
            yyjson_mut_obj_add_uint(doc, files, "total", item->filesTotal);
            yyjson_mut_obj_add_uint(doc, files, "used", item->filesUsed);
        }

        yyjson_mut_obj_add_strbuf(doc, obj, "filesystem", &item->filesystem);
        yyjson_mut_obj_add_strbuf(doc, obj, "mountpoint", &item->mountpoint);
        yyjson_mut_obj_add_strbuf(doc, obj, "mountFrom", &item->mountFrom);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_val* typeArr = yyjson_mut_obj_add_arr(doc, obj, "volumeType");
        if(item->type & FF_DISK_VOLUME_TYPE_REGULAR_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Regular");
        if(item->type & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "External");
        if(item->type & FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Subvolume");
        if(item->type & FF_DISK_VOLUME_TYPE_HIDDEN_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Hidden");
        if(item->type & FF_DISK_VOLUME_TYPE_READONLY_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Read-only");
        if(item->type & FF_DISK_VOLUME_TYPE_UNKNOWN_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Unknown");

        const char* pstr = ffTimeToFullStr(item->createTime);
        if (*pstr)
            yyjson_mut_obj_add_strcpy(doc, obj, "createTime", pstr);
        else
            yyjson_mut_obj_add_null(doc, obj, "createTime");
    }

    FF_LIST_FOR_EACH(FFDisk, item, disks)
    {
        ffStrbufDestroy(&item->mountpoint);
        ffStrbufDestroy(&item->mountFrom);
        ffStrbufDestroy(&item->filesystem);
        ffStrbufDestroy(&item->name);
    }
}

void ffPrintDiskHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_DISK_MODULE_NAME, "{1} / {2} ({3}) - {9}", FF_DISK_NUM_FORMAT_ARGS, ((const char* []) {
        "Size used - size-used",
        "Size total - size-total",
        "Size percentage num - size-percentage",
        "Files used - files-used",
        "Files total - files-total",
        "Files percentage num - files-percentage",
        "True if external volume - is-external",
        "True if hidden volume - is-hidden",
        "Filesystem - filesystem",
        "Label / name - name",
        "True if read-only - is-readonly",
        "Create time in local timezone - create-time",
        "Size percentage bar - size-percentage-bar",
        "Files percentage bar - files-percentage-bar",
    }));
}

void ffInitDiskOptions(FFDiskOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DISK_MODULE_NAME,
        "Print partitions, space usage, file system, etc",
        ffParseDiskCommandOptions,
        ffParseDiskJsonObject,
        ffPrintDisk,
        ffGenerateDiskJsonResult,
        ffPrintDiskHelpFormat,
        ffGenerateDiskJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");

    ffStrbufInit(&options->folders);
    options->showTypes = FF_DISK_VOLUME_TYPE_REGULAR_BIT | FF_DISK_VOLUME_TYPE_EXTERNAL_BIT | FF_DISK_VOLUME_TYPE_READONLY_BIT;
    options->calcType = FF_DISK_CALC_TYPE_FREE;
    options->percent = (FFColorRangeConfig) { 50, 80 };
}

void ffDestroyDiskOptions(FFDiskOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
