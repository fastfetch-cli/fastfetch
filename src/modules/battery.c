#include "fastfetch.h"

#include <unistd.h>
#include <dirent.h>

#define FF_BATTERY_MODULE_NAME "Battery"
#define FF_BATTERY_NUM_FORMAT_ARGS 5

typedef struct BatteryResult
{
    FFstrbuf manufacturer;
    FFstrbuf modelName;
    FFstrbuf technology;
    FFstrbuf capacity;
    FFstrbuf status;
} BatteryResult;

static void parseBattery(FFstrbuf* dir, FFlist* results)
{
    uint32_t dirLength = dir->length;

    FFstrbuf testBatteryBuffer;
    ffStrbufInit(&testBatteryBuffer);

    //type must exist and be "Battery"
    ffStrbufAppendS(dir, "/type");
    ffGetFileContent(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&testBatteryBuffer, "Battery") != 0)
    {
        ffStrbufDestroy(&testBatteryBuffer);
        return;
    }

    //scope may not exist or must not be "Device"
    ffStrbufAppendS(dir, "/scope");
    ffGetFileContent(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&testBatteryBuffer, "Device") == 0)
    {
        ffStrbufDestroy(&testBatteryBuffer);
        return;
    }

    ffStrbufDestroy(&testBatteryBuffer);
    BatteryResult* result = ffListAdd(results);

    //capacity must exist and be not empty
    ffStrbufInit(&result->capacity);
    ffStrbufAppendS(dir, "/capacity");
    ffGetFileContent(dir->chars, &result->capacity);
    ffStrbufSubstrBefore(dir, dirLength);

    if(result->capacity.length == 0)
    {
        ffStrbufDestroy(&result->capacity);
        --results->length;
        return;
    }

    //At this point, we have a battery. Try to get as much values as possible.

    ffStrbufInit(&result->manufacturer);
    ffStrbufAppendS(dir, "/manufacturer");
    ffGetFileContent(dir->chars, &result->manufacturer);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->modelName);
    ffStrbufAppendS(dir, "/model_name");
    ffGetFileContent(dir->chars, &result->modelName);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->technology);
    ffStrbufAppendS(dir, "/technology");
    ffGetFileContent(dir->chars, &result->technology);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->status);
    ffStrbufAppendS(dir, "/status");
    ffGetFileContent(dir->chars, &result->status);
    ffStrbufSubstrBefore(dir, dirLength);
}

static void printBattery(FFinstance* instance, const BatteryResult* result, uint8_t index)
{
    if(instance->config.batteryFormat.length == 0)
    {

        ffPrintLogoAndKey(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey);

        bool showStatus =
            result->status.length > 0 &&
            ffStrbufIgnCaseCompS(&result->status, "Unknown") != 0;

        if(result->capacity.length > 0)
        {
            ffStrbufWriteTo(&result->capacity, stdout);
            putchar('%');

            if(showStatus)
                fputs(" [", stdout);
        }

        if(showStatus)
        {
            ffStrbufWriteTo(&result->status, stdout);

            if(result->capacity.length > 0)
                putchar(']');
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey, &instance->config.batteryFormat, NULL, FF_BATTERY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufacturer},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->modelName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->technology},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->capacity},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->status}
        });
    }
}

void ffPrintBattery(FFinstance* instance)
{
    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);
    if(instance->config.batteryDir.length > 0)
    {
        ffStrbufAppend(&baseDir, &instance->config.batteryDir);
        if(!ffStrbufEndsWithC(&baseDir, '/'))
            ffStrbufAppendC(&baseDir, '/');
    }
    else
    {
        ffStrbufAppendS(&baseDir, "/sys/class/power_supply/");
    }

    uint32_t baseDirLength = baseDir.length;

    DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "opendir(\"%s\") == NULL", baseDir.chars);
        ffStrbufDestroy(&baseDir);
        return;
    }

    FFlist results;
    ffListInitA(&results, sizeof(BatteryResult), 4);

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        ffStrbufAppendS(&baseDir, entry->d_name);
        parseBattery(&baseDir, &results);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    closedir(dirp);

    for(uint8_t i = 0; i < (uint8_t) results.length; i++)
    {
        BatteryResult* result = ffListGet(&results, i);
        printBattery(instance, result, i);

        ffStrbufDestroy(&result->manufacturer);
        ffStrbufDestroy(&result->modelName);
        ffStrbufDestroy(&result->technology);
        ffStrbufDestroy(&result->capacity);
        ffStrbufDestroy(&result->status);
    }

    if(results.length == 0)
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "%s doesn't contain any battery folder", baseDir.chars);

    ffListDestroy(&results);
    ffStrbufDestroy(&baseDir);
}
