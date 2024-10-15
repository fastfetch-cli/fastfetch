#include "battery.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

// https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power

static bool checkAc(const char* id, FFstrbuf* tmpBuffer)
{
    if (ffStrStartsWith(id, "BAT"))
        ffStrbufSetS(tmpBuffer, "/sys/class/power_supply/ADP1/online");
    else if (ffStrStartsWith(id, "macsmc-battery"))
        ffStrbufSetS(tmpBuffer, "/sys/class/power_supply/macsmc-ac/online");
    else
        ffStrbufClear(tmpBuffer);

    char online = '\0';
    return ffReadFileData(tmpBuffer->chars, 1, &online) == 1 && online == '1';
}

static void parseBattery(int dfd, const char* id, FFBatteryOptions* options, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY tmpBuffer = ffStrbufCreate();

    // type must exist and be "Battery"
    if (!ffReadFileBufferRelative(dfd, "type", &tmpBuffer))
        return;
    ffStrbufTrimRightSpace(&tmpBuffer);
    if(!ffStrbufIgnCaseEqualS(&tmpBuffer, "Battery"))
        return;

    // scope may not exist or must not be "Device"
    if (ffReadFileBufferRelative(dfd, "scope", &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);

    if(ffStrbufIgnCaseEqualS(&tmpBuffer, "Device"))
        return;

    // capacity must exist and be not empty
    // This is expensive in my laptop
    if (!ffReadFileBufferRelative(dfd, "capacity", &tmpBuffer))
        return;

    FFBatteryResult* result = ffListAdd(results);
    result->capacity = ffStrbufToDouble(&tmpBuffer);

    //At this point, we have a battery. Try to get as much values as possible.

    ffStrbufInit(&result->manufacturer);
    if (ffReadFileBufferRelative(dfd, "manufacturer", &result->manufacturer))
        ffStrbufTrimRightSpace(&result->manufacturer);
    else if (ffStrEquals(id, "macsmc-battery")) // asahi
        ffStrbufSetStatic(&result->manufacturer, "Apple Inc.");

    ffStrbufInit(&result->modelName);
    if (ffReadFileBufferRelative(dfd, "model_name", &result->modelName))
        ffStrbufTrimRightSpace(&result->modelName);

    ffStrbufInit(&result->technology);
    if (ffReadFileBufferRelative(dfd, "technology", &result->technology))
        ffStrbufTrimRightSpace(&result->technology);

    ffStrbufInit(&result->status);
    if (ffReadFileBufferRelative(dfd, "status", &result->status))
        ffStrbufTrimRightSpace(&result->status);

    // Unknown, Charging, Discharging, Not charging, Full

    result->timeRemaining = -1;
    if (ffStrbufEqualS(&result->status, "Discharging"))
    {
        if (ffReadFileBufferRelative(dfd, "time_to_empty_now", &tmpBuffer))
            result->timeRemaining = (int32_t) ffStrbufToSInt(&tmpBuffer, 0);
        else
        {
            if (ffReadFileBufferRelative(dfd, "charge_now", &tmpBuffer))
            {
                int64_t chargeNow = ffStrbufToSInt(&tmpBuffer, 0);
                if (chargeNow > 0)
                {
                    if (ffReadFileBufferRelative(dfd, "current_now", &tmpBuffer))
                    {
                        int64_t currentNow = ffStrbufToSInt(&tmpBuffer, INT64_MIN);
                        if (currentNow < 0) currentNow = -currentNow;
                        if (currentNow > 0)
                            result->timeRemaining = (int32_t) ((chargeNow * 3600) / currentNow);
                    }
                }
            }
        }

        if (checkAc(id, &tmpBuffer))
            ffStrbufAppendS(&result->status, ", AC Connected");
    }
    else if (ffStrbufEqualS(&result->status, "Not charging") || ffStrbufEqualS(&result->status, "Full"))
        ffStrbufSetStatic(&result->status, "AC Connected");
    else if (ffStrbufEqualS(&result->status, "Charging"))
        ffStrbufAppendS(&result->status, ", AC Connected");
    else if (ffStrbufEqualS(&result->status, "Unknown"))
    {
        ffStrbufClear(&result->status);
        if (checkAc(id, &tmpBuffer))
            ffStrbufAppendS(&result->status, "AC Connected");
    }

    if (ffReadFileBufferRelative(dfd, "capacity_level", &tmpBuffer))
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

    ffStrbufInit(&result->serial);
    if (ffReadFileBufferRelative(dfd, "serial_number", &result->serial))
        ffStrbufTrimRightSpace(&result->serial);

    if (ffReadFileBufferRelative(dfd, "cycle_count", &tmpBuffer))
    {
        int64_t cycleCount = ffStrbufToSInt(&tmpBuffer, 0);
        result->cycleCount = cycleCount < 0 || cycleCount > UINT32_MAX ? 0 : (uint32_t) cycleCount;
    }

    ffStrbufInit(&result->manufactureDate);
    if (ffReadFileBufferRelative(dfd, "manufacture_year", &tmpBuffer))
    {
        int year = (int) ffStrbufToSInt(&tmpBuffer, 0);
        if (year > 0)
        {
            if (ffReadFileBufferRelative(dfd, "manufacture_month", &tmpBuffer))
            {
                int month = (int) ffStrbufToSInt(&tmpBuffer, 0);
                if (month > 0)
                {
                    if (ffReadFileBufferRelative(dfd, "manufacture_day", &tmpBuffer))
                    {
                        int day = (int) ffStrbufToSInt(&tmpBuffer, 0);
                        if (day > 0)
                            ffStrbufSetF(&result->manufactureDate, "%.4d-%.2d-%.2d", year, month, day);
                    }
                }
            }
        }
    }

    result->temperature = FF_BATTERY_TEMP_UNSET;
    if (options->temp)
    {
        if (ffReadFileBufferRelative(dfd, "temp", &tmpBuffer))
            result->temperature = ffStrbufToDouble(&tmpBuffer) / 10;
    }
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/power_supply/");
    if(dirp == NULL)
        return "opendir(\"/sys/class/power_supply/\") == NULL";

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        FF_AUTO_CLOSE_FD int dfd = openat(dirfd(dirp), entry->d_name, O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
        if (dfd > 0) parseBattery(dfd, entry->d_name, options, results);
    }

    return NULL;
}
