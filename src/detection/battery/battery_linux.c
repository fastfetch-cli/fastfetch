#include "fastfetch.h"
#include "common/io/io.h"
#include "battery.h"

#include <dirent.h>

static void parseBattery(FFstrbuf* dir, FFlist* results)
{
    uint32_t dirLength = dir->length;

    FFstrbuf testBatteryBuffer;
    ffStrbufInit(&testBatteryBuffer);

    //type must exist and be "Battery"
    ffStrbufAppendS(dir, "/type");
    ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&testBatteryBuffer, "Battery") != 0)
    {
        ffStrbufDestroy(&testBatteryBuffer);
        return;
    }

    //scope may not exist or must not be "Device"
    ffStrbufAppendS(dir, "/scope");
    ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&testBatteryBuffer, "Device") == 0)
    {
        ffStrbufDestroy(&testBatteryBuffer);
        return;
    }

    BatteryResult* result = ffListAdd(results);

    //capacity must exist and be not empty
    ffStrbufAppendS(dir, "/capacity");
    bool available = ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);
    if(available)
        result->capacity = ffStrbufToDouble(&testBatteryBuffer);
    ffStrbufDestroy(&testBatteryBuffer);
    if(!available)
    {
        result->capacity = 0.0/0.0;
        --results->length;
        return;
    }

    //At this point, we have a battery. Try to get as much values as possible.

    ffStrbufInit(&result->manufacturer);
    ffStrbufAppendS(dir, "/manufacturer");
    ffReadFileBuffer(dir->chars, &result->manufacturer);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->modelName);
    ffStrbufAppendS(dir, "/model_name");
    ffReadFileBuffer(dir->chars, &result->modelName);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->technology);
    ffStrbufAppendS(dir, "/technology");
    ffReadFileBuffer(dir->chars, &result->technology);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->status);
    ffStrbufAppendS(dir, "/status");
    ffReadFileBuffer(dir->chars, &result->status);
    ffStrbufSubstrBefore(dir, dirLength);

    result->temperature = FF_BATTERY_TEMP_UNSET;
}

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results) {
    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);
    if(instance->config.batteryDir.length > 0)
    {
        ffStrbufAppend(&baseDir, &instance->config.batteryDir);
        ffStrbufEnsureEndsWithC(&baseDir, '/');
    }
    else
    {
        ffStrbufAppendS(&baseDir, "/sys/class/power_supply/");
    }

    uint32_t baseDirLength = baseDir.length;

    DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
    {
        ffStrbufDestroy(&baseDir);
        return "opendir(batteryDir) == NULL";
    }

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        parseBattery(&baseDir, results);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    closedir(dirp);

    if(results->length == 0) {
        ffStrbufDestroy(&baseDir);
        return "batteryDir doesn't contain any battery folder";
    }

    ffStrbufDestroy(&baseDir);
    return NULL;
}
