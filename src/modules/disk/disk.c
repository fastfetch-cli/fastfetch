#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/bar.h"
#include "detection/disk/disk.h"
#include "modules/disk/disk.h"

#define FF_DISK_NUM_FORMAT_ARGS 10
#pragma GCC diagnostic ignored "-Wsign-conversion"

static void printDisk(FFinstance* instance, FFDiskOptions* options, const FFDisk* disk)
{
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if(options->moduleArgs.key.length == 0)
    {
        ffStrbufAppendF(&key, "%s (%s)", FF_DISK_MODULE_NAME, disk->mountpoint.chars);
    }
    else
    {
        ffParseFormatString(&key, &options->moduleArgs.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &disk->mountpoint}
        });
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffParseSize(disk->bytesUsed, instance->config.binaryPrefixType, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffParseSize(disk->bytesTotal, instance->config.binaryPrefixType, &totalPretty);

    uint8_t bytesPercentage = disk->bytesTotal > 0 ? (uint8_t) (((long double) disk->bytesUsed / (long double) disk->bytesTotal) * 100.0) : 0;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key.chars, 0, NULL);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

        if(disk->bytesTotal > 0)
        {
            if(instance->config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffAppendPercentBar(instance, &str, bytesPercentage, 0, 5, 8);
                ffStrbufAppendC(&str, ' ');
            }

            if(!(instance->config.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendF(&str, "%s / %s ", usedPretty.chars, totalPretty.chars);

            if(instance->config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                ffAppendPercentNum(instance, &str, (uint8_t) bytesPercentage, 50, 80, str.length > 0);
                ffStrbufAppendC(&str, ' ');
            }
        }
        else
            ffStrbufAppendS(&str, "Unknown ");

        if(!(instance->config.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
        {
            ffStrbufAppendF(&str, "- %s ", disk->filesystem.chars);

            if(disk->type & FF_DISK_TYPE_EXTERNAL_BIT)
                ffStrbufAppendS(&str, "[External]");
            else if(disk->type & FF_DISK_TYPE_SUBVOLUME_BIT)
                ffStrbufAppendS(&str, "[Subvolume]");
            else if(disk->type & FF_DISK_TYPE_HIDDEN_BIT)
                ffStrbufAppendS(&str, "[Hidden]");
        }

        ffStrbufTrimRight(&str, ' ');
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        uint8_t filesPercentage = disk->filesTotal > 0 ? (uint8_t) (((double) disk->filesUsed / (double) disk->filesTotal) * 100.0) : 0;

        bool isExternal = !!(disk->type & FF_DISK_TYPE_EXTERNAL_BIT);
        bool isHidden = !!(disk->type & FF_DISK_TYPE_HIDDEN_BIT);
        ffPrintFormatString(instance, key.chars, 0, NULL, &options->moduleArgs.outputFormat, FF_DISK_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &bytesPercentage},
            {FF_FORMAT_ARG_TYPE_UINT, &disk->filesUsed},
            {FF_FORMAT_ARG_TYPE_UINT, &disk->filesTotal},
            {FF_FORMAT_ARG_TYPE_UINT8, &filesPercentage},
            {FF_FORMAT_ARG_TYPE_BOOL, &isExternal},
            {FF_FORMAT_ARG_TYPE_BOOL, &isHidden},
            {FF_FORMAT_ARG_TYPE_STRBUF, &disk->filesystem},
            {FF_FORMAT_ARG_TYPE_STRBUF, &disk->name}
        });
    }
}

static void printMountpoint(FFinstance* instance, FFDiskOptions* options, const FFlist* disks, const char* mountpoint)
{
    for(uint32_t i = disks->length; i > 0; i--)
    {
        FFDisk* disk = ffListGet(disks, i - 1);
        if(strncmp(mountpoint, disk->mountpoint.chars, disk->mountpoint.length) == 0)
        {
            printDisk(instance, options, disk);
            return;
        }
    }

    ffPrintError(instance, FF_DISK_MODULE_NAME, 0, &options->moduleArgs, "No disk found for mountpoint: %s", mountpoint);
}

static void printMountpoints(FFinstance* instance, FFDiskOptions* options, const FFlist* disks)
{
    #ifdef _WIN32
    const char separator = ';';
    #else
    const char separator = ':';
    #endif

    FF_STRBUF_AUTO_DESTROY mountpoints = ffStrbufCreateCopy(&options->folders);
    ffStrbufTrim(&mountpoints, separator);

    uint32_t startIndex = 0;
    while(startIndex < mountpoints.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&mountpoints, startIndex, separator);
        mountpoints.chars[colonIndex] = '\0';

        printMountpoint(instance, options, disks, mountpoints.chars + startIndex);

        startIndex = colonIndex + 1;
    }
}

static void printAutodetected(FFinstance* instance, FFDiskOptions* options, const FFlist* disks)
{
    FF_LIST_FOR_EACH(FFDisk, disk, *disks)
    {
        if(!(disk->type & options->showTypes))
            continue;

        printDisk(instance, options, disk);
    }
}

void ffPrintDisk(FFinstance* instance, FFDiskOptions* options)
{
    FF_LIST_AUTO_DESTROY disks = ffListCreate(sizeof (FFDisk));
    const char* error = ffDetectDisks(&disks);

    if(error)
    {
        ffPrintError(instance, FF_DISK_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
    }
    else
    {
        if(options->folders.length == 0)
            printAutodetected(instance, options, &disks);
        else
            printMountpoints(instance, options, &disks);
    }

    FF_LIST_FOR_EACH(FFDisk, disk, disks)
    {
        ffStrbufDestroy(&disk->mountpoint);
        ffStrbufDestroy(&disk->filesystem);
        ffStrbufDestroy(&disk->name);
    }
}


void ffInitDiskOptions(FFDiskOptions* options)
{
    options->moduleName = FF_DISK_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInit(&options->folders);
    options->showTypes = FF_DISK_TYPE_REGULAR_BIT | FF_DISK_TYPE_EXTERNAL_BIT;
}

bool ffParseDiskCommandOptions(FFDiskOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DISK_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "folders") == 0)
    {
        ffOptionParseString(key, value, &options->folders);
        return true;
    }

    if (strcasecmp(subKey, "show-regular") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_TYPE_REGULAR_BIT;
        else
            options->showTypes &= ~FF_DISK_TYPE_REGULAR_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-external") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_TYPE_EXTERNAL_BIT;
        else
            options->showTypes &= ~FF_DISK_TYPE_EXTERNAL_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-hidden") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_TYPE_HIDDEN_BIT;
        else
            options->showTypes &= ~FF_DISK_TYPE_HIDDEN_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-subvolumes") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_TYPE_SUBVOLUME_BIT;
        else
            options->showTypes &= ~FF_DISK_TYPE_SUBVOLUME_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-unknown") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showTypes |= FF_DISK_TYPE_UNKNOWN_BIT;
        else
            options->showTypes &= ~FF_DISK_TYPE_UNKNOWN_BIT;
        return true;
    }

    return false;
}

void ffDestroyDiskOptions(FFDiskOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseDiskJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFDiskOptions __attribute__((__cleanup__(ffDestroyDiskOptions))) options;
    ffInitDiskOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "folders") == 0)
            {
                ffStrbufSetS(&options.folders, yyjson_get_str(val));
                continue;
            }

            if (strcasecmp(key, "showExternal") == 0)
            {
                if (yyjson_get_bool(val))
                    options.showTypes |= FF_DISK_TYPE_EXTERNAL_BIT;
                else
                    options.showTypes &= ~FF_DISK_TYPE_EXTERNAL_BIT;
                continue;
            }

            if (strcasecmp(key, "showHidden") == 0)
            {
                if (yyjson_get_bool(val))
                    options.showTypes |= FF_DISK_TYPE_HIDDEN_BIT;
                else
                    options.showTypes &= ~FF_DISK_TYPE_HIDDEN_BIT;
                continue;
            }

            if (strcasecmp(key, "showSubvolumes") == 0)
            {
                if (yyjson_get_bool(val))
                    options.showTypes |= FF_DISK_TYPE_SUBVOLUME_BIT;
                else
                    options.showTypes &= ~FF_DISK_TYPE_SUBVOLUME_BIT;
                continue;
            }

            if (strcasecmp(key, "show-unknown") == 0)
            {
                if (yyjson_get_bool(val))
                    options.showTypes |= FF_DISK_TYPE_UNKNOWN_BIT;
                else
                    options.showTypes &= ~FF_DISK_TYPE_UNKNOWN_BIT;
                continue;
            }

            ffPrintError(instance, FF_DISK_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintDisk(instance, &options);
}
