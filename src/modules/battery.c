#include "fastfetch.h"

#include <unistd.h>
#include <dirent.h>

#define FF_BATTERY_MODULE_NAME "Battery"
#define FF_BATTERY_NUM_FORMAT_ARGS 5

static void printBattery(FFinstance* instance, FFstrbuf* dir, uint8_t index)
{
    uint32_t dirLength = dir->length;

    FF_STRBUF_CREATE(manufactor);
    ffStrbufAppendS(dir, "/manufacturer");
    ffGetFileContent(dir->chars, &manufactor);
    ffStrbufSubstrBefore(dir, dirLength);

    FF_STRBUF_CREATE(model);
    ffStrbufAppendS(dir, "/model_name");
    ffGetFileContent(dir->chars, &model);
    ffStrbufSubstrBefore(dir, dirLength);

    FF_STRBUF_CREATE(technology);
    ffStrbufAppendS(dir, "/technology");
    ffGetFileContent(dir->chars, &technology);
    ffStrbufSubstrBefore(dir, dirLength);

    FF_STRBUF_CREATE(capacity);
    ffStrbufAppendS(dir, "/capacity");
    ffGetFileContent(dir->chars, &capacity);
    ffStrbufSubstrBefore(dir, dirLength);

    FF_STRBUF_CREATE(status);
    ffStrbufAppendS(dir, "/status");
    ffGetFileContent(dir->chars, &status);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&status, "Unknown") == 0)
        ffStrbufClear(&status);

    if(capacity.length == 0 && status.length == 0)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "No file in %s could be read or all battery options are disabled", dir->chars);
        return;
    }

    if(instance->config.batteryFormat.length == 0)
    {

        ffPrintLogoAndKey(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey);

        bool showStatus = status.length > 0 && ffStrbufIgnCaseCompS(&status, "Full") != 0;

        if(capacity.length > 0)
        {
            ffStrbufWriteTo(&capacity, stdout);
            putchar('%');

            if(showStatus)
                fputs(" [", stdout);
        }

        if(showStatus)
        {
            ffStrbufWriteTo(&status, stdout);

            if(capacity.length > 0)
                putchar(']');
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey, &instance->config.batteryFormat, NULL, FF_BATTERY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &manufactor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &model},
            {FF_FORMAT_ARG_TYPE_STRBUF, &technology},
            {FF_FORMAT_ARG_TYPE_STRBUF, &capacity},
            {FF_FORMAT_ARG_TYPE_STRBUF, &status}
        });
    }

    ffStrbufDestroy(&manufactor);
    ffStrbufDestroy(&model);
    ffStrbufDestroy(&technology);
    ffStrbufDestroy(&capacity);
    ffStrbufDestroy(&status);
}

void ffPrintBattery(FFinstance* instance)
{
    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);
    if(instance->config.batteryDir.length > 0)
    {
        ffStrbufAppend(&baseDir, &instance->config.batteryDir);

        if(baseDir.length == 0)
        {
            ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "custom battery dir is an empty string");
            ffStrbufDestroy(&baseDir);
            return;
        }

        if(baseDir.chars[baseDir.length - 1] != '/')
            ffStrbufAppendC(&baseDir, '/');
    }
    else
        ffStrbufAppendS(&baseDir, "/sys/class/power_supply/");

    uint32_t baseDirLength = baseDir.length;

    DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "opendir(\"%s\") == NULL", baseDir.chars);
        ffStrbufDestroy(&baseDir);
        return;
    }

    FFlist dirs;
    ffListInitA(&dirs, sizeof(FFstrbuf), 4);

    struct dirent* entry;

    while((entry = readdir(dirp)) != NULL)
    {
        ffStrbufAppendS(&baseDir, entry->d_name);
        ffStrbufAppendS(&baseDir, "/capacity");

        if(access(baseDir.chars, F_OK) == 0)
        {
            FFstrbuf* name = ffListAdd(&dirs);
            ffStrbufInit(name);
            ffStrbufSetS(name, entry->d_name);
        }

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    if(dirs.length == 0)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "%s doesn't contain any battery folder", baseDir.chars);
        ffListDestroy(&dirs);
        ffStrbufDestroy(&baseDir);
        return;
    }

    for(uint8_t i = 0; i < dirs.length; i++)
    {
        FFstrbuf* name = ffListGet(&dirs, i);
        ffStrbufAppend(&baseDir, name);
        printBattery(instance, &baseDir, dirs.length == 1 ? 0 : i + 1);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);
        ffStrbufDestroy(name);
    }

    ffListDestroy(&dirs);
    ffStrbufDestroy(&baseDir);
}
