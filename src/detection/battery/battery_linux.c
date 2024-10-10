#include "battery.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <dirent.h>

// https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power

static inline const char* findProperty(const char* buffer, ssize_t bufSize, const char* prefix)
{
    assert(bufSize > 0);
    const char* p = memmem(buffer, (size_t) bufSize, prefix, strlen(prefix));
    if (p) return p + strlen(prefix);
    return NULL;
}

static void parseBattery(FFstrbuf* dir, const char* id, FFBatteryOptions* options, FFlist* results)
{
    ffStrbufAppendS(dir, "/uevent");

    char buffer[16 * 1024];
    ssize_t size = ffReadFileData(dir->chars, sizeof(buffer), buffer);
    if (size <= 0) return;

    //type must exist and be "Battery"
    const char* type = findProperty(buffer, size, "\nPOWER_SUPPLY_TYPE=");
    if (type)
    {
        if (!ffStrStartsWith(type, "Battery\n")) return;
    }
    else if (!ffStrStartsWith(id, "BAT"))
    {
        char typeBuf[16] = {0};
        ffStrbufSubstrBefore(dir, dir->length - (uint32_t) strlen("uevent"));
        ffStrbufAppendS(dir, "type");
        if (ffReadFileData(dir->chars, sizeof(typeBuf), typeBuf) < (ssize_t) strlen("Battery")) return;
        if (!ffStrStartsWith(typeBuf, "Battery")) return;
    }

    //scope may not exist or must not be "Device"
    const char* scope = findProperty(buffer, size, "\nPOWER_SUPPLY_SCOPE=");
    if (scope && ffStrStartsWith(scope, "Device\n")) return;

    //capacity must exist and be not empty
    const char* capacity = findProperty(buffer, size, "\nPOWER_SUPPLY_CAPACITY=");
    if (!capacity) return;

    //At this point, we have a battery. Try to get as much values as possible.

    FFBatteryResult* result = ffListAdd(results);
    result->capacity = strtod(capacity, NULL);

    ffStrbufInit(&result->manufacturer);
    const char* manufacturer = findProperty(buffer, size, "\nPOWER_SUPPLY_MANUFACTURER=");
    if (manufacturer)
        ffStrbufAppendSUntilC(&result->manufacturer, manufacturer, '\n');
    else if (ffStrEquals(id, "macsmc-battery")) // asahi
        ffStrbufSetStatic(&result->manufacturer, "Apple Inc.");

    ffStrbufInit(&result->modelName);
    ffStrbufAppendSUntilC(&result->modelName, findProperty(buffer, size, "\nPOWER_SUPPLY_MODEL_NAME="), '\n');

    ffStrbufInit(&result->technology);
    const char* technology = findProperty(buffer, size, "\nPOWER_SUPPLY_TECHNOLOGY=");
    if (technology && !ffStrStartsWith(technology, "Unknown"))
        ffStrbufAppendSUntilC(&result->technology, technology, '\n');

    ffStrbufInit(&result->status);
    const char* status = findProperty(buffer, size, "\nPOWER_SUPPLY_STATUS=");
    if (status)
    {
        // Unknown, Charging, Discharging, Not charging, Full
        if (ffStrStartsWith(status, "Not charging\n") || ffStrStartsWith(status, "Full\n"))
            ffStrbufSetStatic(&result->status, "AC Connected");
        else if (ffStrStartsWith(status, "Charging\n"))
            ffStrbufSetStatic(&result->status, "AC Connected, Charging");
        else if (ffStrStartsWith(status, "Discharging\n"))
            ffStrbufSetStatic(&result->status, "Discharging");
    }

    const char* capacityLevel = findProperty(buffer, size, "\nPOWER_SUPPLY_CAPACITY_LEVEL=");
    if (capacityLevel && ffStrStartsWith(capacityLevel, "Critical\n"))
    {
        if (result->status.length)
            ffStrbufAppendS(&result->status, ", Critical");
        else
            ffStrbufSetStatic(&result->status, "Critical");
    }

    ffStrbufInit(&result->serial);
    ffStrbufAppendSUntilC(&result->serial, findProperty(buffer, size, "\nPOWER_SUPPLY_SERIAL_NUMBER="), '\n');

    ffStrbufAppendS(dir, "/cycle_count");
    const char* cycleCountStr = findProperty(buffer, size, "\nPOWER_SUPPLY_CYCLE_COUNT=");
    if (cycleCountStr)
    {
        int64_t cycleCount = strtol(cycleCountStr, NULL, 10);
        result->cycleCount = cycleCount < 0 || cycleCount > UINT32_MAX ? 0 : (uint32_t) cycleCount;
    }

    result->timeRemaining = -1;
    const char* timeToEmptyStr = findProperty(buffer, size, "\nPOWER_SUPPLY_TIME_TO_EMPTY_NOW=");
    if (timeToEmptyStr)
    {
        int64_t value = strtol(timeToEmptyStr, NULL, 0);
        if (value > 0)
            result->timeRemaining = (int32_t) value;
        else
        {
            const char* timeToFullStr = findProperty(buffer, size, "\nPOWER_SUPPLY_TIME_TO_FULL_NOW=");
            if (timeToFullStr)
            {
                value = strtol(timeToFullStr, NULL, 0);
                if (value > 0)
                    result->timeRemaining = (int32_t) value;
            }
        }
    }
    else if (ffStrStartsWith(status, "Discharging\n"))
    {
        const char* chargeNow = findProperty(buffer, size, "\nPOWER_SUPPLY_CHARGE_NOW=");
        if (chargeNow)
        {
            int64_t charge = strtol(chargeNow, NULL, 0);
            if (charge > 0)
            {
                const char* currentNow = findProperty(buffer, size, "\nPOWER_SUPPLY_CURRENT_NOW=");
                if (currentNow)
                {
                    int64_t current = strtol(currentNow, NULL, 0);
                    if (current > 0)
                        result->timeRemaining = (int32_t) (charge * 3600 / current);
                }
            }
        }
    }

    ffStrbufInit(&result->manufactureDate);
    const char* myear = findProperty(buffer, size, "\nPOWER_SUPPLY_MANUFACTURE_YEAR=");
    if (myear)
    {
        int year = (int) strtol(myear, NULL, 10);
        if (year > 0)
        {
            const char* mmonth = findProperty(buffer, size, "\nPOWER_SUPPLY_MANUFACTURE_MONTH=");
            if (mmonth)
            {
                int month = (int) strtol(mmonth, NULL, 10);
                if (month > 0)
                {
                    const char* mday = findProperty(buffer, size, "\nPOWER_SUPPLY_MANUFACTURE_DAY=");
                    if (mday)
                    {
                        int day = (int) strtol(mday, NULL, 0);
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
        const char* tempStr = findProperty(buffer, size, "\nPOWER_SUPPLY_TEMP=");
        if (tempStr)
            result->temperature = strtod(tempStr, NULL) / 10.;
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
