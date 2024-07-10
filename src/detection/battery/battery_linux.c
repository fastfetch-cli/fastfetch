#include "battery.h"
#include "common/io/io.h"
#include "detection/temps/temps_linux.h"
#include "util/stringUtils.h"

#include <dirent.h>

// https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power

static void parseBattery(FFstrbuf* dir, const char* id, FFBatteryOptions* options, FFlist* results)
{
    uint32_t dirLength = dir->length;

    FF_STRBUF_AUTO_DESTROY tmpBuffer = ffStrbufCreate();

    //type must exist and be "Battery"
    ffStrbufAppendS(dir, "/type");
    if (ffReadFileBuffer(dir->chars, &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(!ffStrbufIgnCaseEqualS(&tmpBuffer, "Battery"))
        return;

    //scope may not exist or must not be "Device"
    ffStrbufAppendS(dir, "/scope");
    if (ffReadFileBuffer(dir->chars, &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseEqualS(&tmpBuffer, "Device"))
        return;

    //capacity must exist and be not empty
    ffStrbufAppendS(dir, "/capacity");
    bool available = ffReadFileBuffer(dir->chars, &tmpBuffer); // This is expensive in my laptop
    ffStrbufSubstrBefore(dir, dirLength);

    if (!available)
        return;

    FFBatteryResult* result = ffListAdd(results);
    result->capacity = ffStrbufToDouble(&tmpBuffer);

    //At this point, we have a battery. Try to get as much values as possible.

    ffStrbufInit(&result->manufacturer);
    ffStrbufAppendS(dir, "/manufacturer");
    if (ffReadFileBuffer(dir->chars, &result->manufacturer))
        ffStrbufTrimRightSpace(&result->manufacturer);
    else if (ffStrEquals(id, "macsmc-battery")) // asahi
        ffStrbufSetStatic(&result->manufacturer, "Apple Inc.");
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->modelName);
    ffStrbufAppendS(dir, "/model_name");
    if (ffReadFileBuffer(dir->chars, &result->modelName))
        ffStrbufTrimRightSpace(&result->modelName);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->technology);
    ffStrbufAppendS(dir, "/technology");
    if (ffReadFileBuffer(dir->chars, &result->technology))
        ffStrbufTrimRightSpace(&result->technology);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->status);
    ffStrbufAppendS(dir, "/status");
    if (ffReadFileBuffer(dir->chars, &result->status))
        ffStrbufTrimRightSpace(&result->status);
    ffStrbufSubstrBefore(dir, dirLength);

    // Unknown, Charging, Discharging, Not charging, Full
    if (ffStrbufEqualS(&result->status, "Not charging") || ffStrbufEqualS(&result->status, "Full"))
        ffStrbufSetStatic(&result->status, "AC Connected");
    else if (ffStrbufEqualS(&result->status, "Unknown"))
        ffStrbufClear(&result->status);

    ffStrbufAppendS(dir, "/capacity_level");
    if (ffReadFileBuffer(dir->chars, &tmpBuffer))
    {
        ffStrbufTrimRightSpace(&result->manufacturer);
        if (ffStrbufEqualS(&tmpBuffer, "Critical"))
        {
            if (result->status.length)
                ffStrbufAppendS(&result->status, ", Critical");
            else
                ffStrbufSetStatic(&result->status, "Critical");
        }
    }
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufInit(&result->serial);
    ffStrbufAppendS(dir, "/serial_number");
    if (ffReadFileBuffer(dir->chars, &result->serial))
        ffStrbufTrimRightSpace(&result->serial);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "/cycle_count");
    available = ffReadFileBuffer(dir->chars, &tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);
    if (available)
    {
        int64_t cycleCount = ffStrbufToSInt(&tmpBuffer, 0);
        result->cycleCount = cycleCount < 0 || cycleCount > UINT32_MAX ? 0 : (uint32_t) cycleCount;
    }

    ffStrbufInit(&result->manufactureDate);
    ffStrbufAppendS(dir, "/manufacture_year");
    available = ffReadFileBuffer(dir->chars, &tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);
    if (available)
    {
        int year = (int) ffStrbufToSInt(&tmpBuffer, 0);
        if (year > 0)
        {
            ffStrbufAppendS(dir, "/manufacture_month");
            available = ffReadFileBuffer(dir->chars, &tmpBuffer);
            ffStrbufSubstrBefore(dir, dirLength);
            if (available)
            {
                int month = (int) ffStrbufToSInt(&tmpBuffer, 0);
                if (month > 0)
                {
                    ffStrbufAppendS(dir, "/manufacture_day");
                    available = ffReadFileBuffer(dir->chars, &tmpBuffer);
                    ffStrbufSubstrBefore(dir, dirLength);
                    if (available)
                    {
                        int day = (int) ffStrbufToSInt(&tmpBuffer, 0);
                        if (day > 0)
                        {
                            ffStrbufSetF(&result->manufactureDate, "%.4d-%.2d-%.2d", year, month, day);
                        }
                    }
                }
            }
        }
    }

    result->temperature = FF_BATTERY_TEMP_UNSET;
    if (options->temp)
    {
        ffStrbufAppendS(dir, "/temp");
        if (ffReadFileBuffer(dir->chars, &tmpBuffer))
            result->temperature = ffStrbufToDouble(&tmpBuffer) / 10;
        ffStrbufSubstrBefore(dir, dirLength);
    }
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&baseDir, "/sys/class/power_supply/");

    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return "opendir(\"/sys/class/power_supply/\") == NULL";

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        parseBattery(&baseDir, entry->d_name, options, results);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    return NULL;
}
