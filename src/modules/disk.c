#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/bar.h"
#include "detection/disk/disk.h"

#define FF_DISK_MODULE_NAME "Disk"
#define FF_DISK_NUM_FORMAT_ARGS 10

static void printDisk(FFinstance* instance, const FFDisk* disk)
{
    FFstrbuf key;
    ffStrbufInit(&key);

    if(instance->config.disk.key.length == 0)
    {
        ffStrbufAppendF(&key, "%s (%s)", FF_DISK_MODULE_NAME, disk->mountpoint.chars);
    }
    else
    {
        ffParseFormatString(&key, &instance->config.disk.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &disk->mountpoint}
        });
    }

    FFstrbuf usedPretty;
    ffStrbufInit(&usedPretty);
    ffParseSize(disk->bytesUsed, instance->config.binaryPrefixType, &usedPretty);

    FFstrbuf totalPretty;
    ffStrbufInit(&totalPretty);
    ffParseSize(disk->bytesTotal, instance->config.binaryPrefixType, &totalPretty);

    uint8_t bytesPercentage = disk->bytesTotal > 0 ? (uint8_t) (((long double) disk->bytesUsed / (long double) disk->bytesTotal) * 100.0) : 0;

    if(instance->config.disk.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key.chars, 0, NULL);

        FFstrbuf str;
        ffStrbufInit(&str);

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

        if(disk->type == FF_DISK_TYPE_EXTERNAL && !(instance->config.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
            ffStrbufAppendS(&str, "[Removable]");

        ffStrbufTrimRight(&str, ' ');
        ffStrbufPutTo(&str, stdout);
        ffStrbufDestroy(&str);
    }
    else
    {
        uint8_t filesPercentage = disk->filesTotal > 0 ? (uint8_t) (((double) disk->filesUsed / (double) disk->filesTotal) * 100.0) : 0;

        ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.disk.outputFormat, FF_DISK_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &bytesPercentage},
            {FF_FORMAT_ARG_TYPE_UINT, &disk->filesUsed},
            {FF_FORMAT_ARG_TYPE_UINT, &disk->filesTotal},
            {FF_FORMAT_ARG_TYPE_UINT8, &filesPercentage},
            {FF_FORMAT_ARG_TYPE_BOOL, FF_FORMAT_ARG_VALUE_BOOL(disk->type == FF_DISK_TYPE_EXTERNAL)},
            {FF_FORMAT_ARG_TYPE_BOOL, FF_FORMAT_ARG_VALUE_BOOL(disk->type == FF_DISK_TYPE_HIDDEN)},
            {FF_FORMAT_ARG_TYPE_STRBUF, &disk->filesystem},
            {FF_FORMAT_ARG_TYPE_STRBUF, &disk->name}
        });
    }

    ffStrbufDestroy(&totalPretty);
    ffStrbufDestroy(&usedPretty);
    ffStrbufDestroy(&key);
}

static void printMountpoint(FFinstance* instance, const FFlist* disks, const char* mountpoint)
{
    for(uint32_t i = disks->length; i > 0; i--)
    {
        FFDisk* disk = ffListGet(disks, i - 1);
        if(strncmp(mountpoint, disk->mountpoint.chars, disk->mountpoint.length) == 0)
        {
            printDisk(instance, disk);
            return;
        }
    }

    ffPrintError(instance, FF_DISK_MODULE_NAME, 0, &instance->config.disk, "No disk found for mountpoint: %s", mountpoint);
}

static void printMountpoints(FFinstance* instance, const FFlist* disks)
{
    #ifdef _WIN32
    const char separator = ';';
    #else
    const char separator = ':';
    #endif

    FFstrbuf mountpoints;
    ffStrbufInitCopy(&mountpoints, &instance->config.diskFolders);
    ffStrbufTrim(&mountpoints, separator);

    uint32_t startIndex = 0;
    while(startIndex < mountpoints.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&mountpoints, startIndex, separator);
        mountpoints.chars[colonIndex] = '\0';

        printMountpoint(instance, disks, mountpoints.chars + startIndex);

        startIndex = colonIndex + 1;
    }
}

static void printAutodetected(FFinstance* instance, const FFlist* disks)
{
    for(uint32_t i = 0; i < disks->length; i++)
    {
        const FFDisk* disk = ffListGet(disks, i);

        if(disk->type == FF_DISK_TYPE_EXTERNAL && !instance->config.diskShowRemovable)
            continue;

        if(disk->type == FF_DISK_TYPE_SUBVOLUME && !instance->config.diskShowSubvolumes)
            continue;

        if(disk->type == FF_DISK_TYPE_HIDDEN && !instance->config.diskShowHidden)
            continue;

        if(disk->bytesTotal == 0 && !instance->config.diskShowUnknown)
            continue;

        printDisk(instance, disk);
    }
}

void ffPrintDisk(FFinstance* instance)
{
    const FFDiskResult* disks = ffDetectDisks();
    if(disks->error.length > 0)
    {
        ffPrintError(instance, FF_DISK_MODULE_NAME, 0, &instance->config.disk, "%s", disks->error.chars);
        return;
    }

    if(instance->config.diskFolders.length == 0)
        printAutodetected(instance, &disks->disks);
    else
        printMountpoints(instance, &disks->disks);
}
