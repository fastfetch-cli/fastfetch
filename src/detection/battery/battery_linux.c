#include "battery.h"
#include "common/io/io.h"
#include "detection/temps/temps_linux.h"
#include "util/stringUtils.h"

#include <dirent.h>

static void parseBattery(FFstrbuf* dir, const char* id, FFBatteryOptions* options, FFlist* results)
{
    uint32_t dirLength = dir->length;

    FF_STRBUF_AUTO_DESTROY testBatteryBuffer = ffStrbufCreate();

    //type must exist and be "Battery"
    ffStrbufAppendS(dir, "/type");
    ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&testBatteryBuffer, "Battery") != 0)
        return;

    //scope may not exist or must not be "Device"
    ffStrbufAppendS(dir, "/scope");
    ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseCompS(&testBatteryBuffer, "Device") == 0)
        return;

    FFBatteryResult* result = ffListAdd(results);

    //capacity must exist and be not empty
    ffStrbufAppendS(dir, "/capacity");
    bool available = ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);
    if(available)
        result->capacity = ffStrbufToDouble(&testBatteryBuffer);

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

    ffStrbufAppendS(dir, "/cycle_count");
    ffReadFileBuffer(dir->chars, &testBatteryBuffer);
    ffStrbufSubstrBefore(dir, dirLength);
    if(dir->length)
        result->cycleCount = (int32_t) ffStrbufToUInt(&testBatteryBuffer, 0);

    result->temperature = FF_BATTERY_TEMP_UNSET;
    if (options->temp)
    {
        const FFlist* tempsResult = ffDetectTemps();

        FF_LIST_FOR_EACH(FFTempValue, value, *tempsResult)
        {
            if (ffStrbufEqualS(&value->name, id))
            {
                result->temperature = value->value;
                break;
            }
        }
    }
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);

    if(options->dir.length > 0)
    {
        ffStrbufAppend(&baseDir, &options->dir);
        ffStrbufEnsureEndsWithC(&baseDir, '/');
    }
    else
    {
        ffStrbufAppendS(&baseDir, "/sys/class/power_supply/");
    }

    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return "opendir(batteryDir) == NULL";

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        parseBattery(&baseDir, entry->d_name, options, results);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    if(results->length == 0)
        return "batteryDir doesn't contain any battery folder";

    return NULL;
}
