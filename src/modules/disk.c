#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "detection/disk/disk.h"

#include <sys/statvfs.h>

#define FF_DISK_MODULE_NAME "Disk"
#define FF_DISK_NUM_FORMAT_ARGS 4

static void printFolder(FFinstance* instance, FFDiskResult* folder)
{
    FFstrbuf key;
    ffStrbufInit(&key);

    if(instance->config.disk.key.length == 0)
    {
        ffStrbufAppendF(&key, "%s (%*s)", FF_DISK_MODULE_NAME, folder->path.length, folder->path.chars);
    }
    else
    {
        ffParseFormatString(&key, &instance->config.disk.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &folder->path}
        });
    }

    uint8_t percentage = (uint8_t) ((folder->used / (long double) folder->total) * 100.0);

    FFstrbuf usedPretty;
    ffStrbufInit(&usedPretty);
    ffParseSize(folder->used, instance->config.binaryPrefixType, &usedPretty);

    FFstrbuf totalPretty;
    ffStrbufInit(&totalPretty);
    ffParseSize(folder->total, instance->config.binaryPrefixType, &totalPretty);

    if(instance->config.disk.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key.chars, 0, NULL);
        printf("%s / %s (%u%%)\n", usedPretty.chars, totalPretty.chars, percentage);
    }
    else
    {
        ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.disk.outputFormat, FF_DISK_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
            {FF_FORMAT_ARG_TYPE_UINT, &folder->files},
        });
    }

    ffStrbufDestroy(&key);
    ffStrbufDestroy(&totalPretty);
    ffStrbufDestroy(&usedPretty);
}

void ffPrintDisk(FFinstance* instance)
{
    FFlist folders;
    ffListInit(&folders, sizeof(FFDiskResult));

    const char* error = NULL;

    if(!ffDiskDetectDiskFolders(instance, &folders))
        error = ffDiskAutodetectFolders(instance, &folders);

    if(error)
    {
        ffPrintError(instance, FF_DISK_MODULE_NAME, 0, &instance->config.disk, "%s", error);
    }
    else
    {
        for(uint32_t i = 0; i < folders.length; ++i)
        {
            FFDiskResult* folder = (FFDiskResult*)ffListGet(&folders, i);
            printFolder(instance, folder);
            ffStrbufDestroy(&folder->path);
            ffStrbufDestroy(&folder->error);
        }
    }

    ffListDestroy(&folders);
}
