#include "battery.h"
#include "common/io.h"
#include "common/stringUtils.h"
#include "common/debug.h"

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

// https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power

static bool parseBattery(int dfd, const char* id, FFBatteryOptions* options, FFlist* results, bool* acConnected) {
    FF_STRBUF_AUTO_DESTROY tmpBuffer = ffStrbufCreate();

    {
        char present = '\0';
        if (ffReadFileDataRelative(dfd, "present", 1, &present) && present == '0') {
            FF_DEBUG("Battery \"%s\": Not present", id);
            return false;
        }
    }

    // type must exist
    if (!ffReadFileBufferRelative(dfd, "type", &tmpBuffer)) {
        FF_DEBUG("Battery \"%s\": No type file", id);
        return false;
    }
    ffStrbufTrimRightSpace(&tmpBuffer);
    if (ffStrbufEqualS(&tmpBuffer, "Mains")) {
        if (*acConnected) {
            FF_DEBUG("Battery \"%s\": Type is Mains, but AC is already connected", id);
            return false;
        }

        char online = '\0';
        if (ffReadFileDataRelative(dfd, "online", 1, &online) == 1 && online == '1') {
            *acConnected = true;
        }
        FF_DEBUG("Battery \"%s\": Type is Mains, AC Connected: %s", id, *acConnected ? "Yes" : "No");
        return false;
    } else if (!ffStrbufEqualS(&tmpBuffer, "Battery")) {
        FF_DEBUG("Battery \"%s\": Type is not Battery or Mains, but \"%s\"", id, tmpBuffer.chars);
        return false;
    }

    // scope may not exist or must not be "Device"
    if (ffReadFileBufferRelative(dfd, "scope", &tmpBuffer)) {
        ffStrbufTrimRightSpace(&tmpBuffer);

        FF_DEBUG("Battery \"%s\": Scope is \"%s\"", id, tmpBuffer.chars);
        if (ffStrbufEqualS(&tmpBuffer, "Device")) {
            return false;
        }
    }

    // `capacity` must exist
    // This is expensive in my laptop
    if (!ffReadFileBufferRelative(dfd, "capacity", &tmpBuffer)) {
        FF_DEBUG("Battery \"%s\": No capacity file", id);
        return false;
    }

    FFBatteryResult* result = ffListAdd(results);
    ffStrbufInit(&result->manufacturer);
    ffStrbufInit(&result->modelName);
    ffStrbufInit(&result->technology);
    ffStrbufInit(&result->serial);
    ffStrbufInit(&result->manufactureDate);
    result->status = FF_BATTERY_STATUS_NONE;
    result->capacity = ffStrbufToDouble(&tmpBuffer, 0);
    result->cycleCount = 0;
    result->temperature = FF_BATTERY_TEMP_UNSET;
    result->timeRemaining = -1;

    // At this point, we have a battery. Try to get as much values as possible.

    if (ffReadFileBufferRelative(dfd, "manufacturer", &result->manufacturer)) {
        ffStrbufTrimRightSpace(&result->manufacturer);
    } else if (ffStrEquals(id, "macsmc-battery")) { // asahi
        ffStrbufSetStatic(&result->manufacturer, "Apple Inc.");
    }

    if (ffReadFileBufferRelative(dfd, "model_name", &result->modelName)) {
        ffStrbufTrimRightSpace(&result->modelName);
    }

    if (ffReadFileBufferRelative(dfd, "technology", &result->technology)) {
        ffStrbufTrimRightSpace(&result->technology);
    }

    if (ffReadFileBufferRelative(dfd, "status", &tmpBuffer)) {
        ffStrbufTrimRightSpace(&tmpBuffer);
    }

    // Unknown, Charging, Discharging, Not charging, Full

    if (ffStrbufEqualS(&tmpBuffer, "Discharging")) {
        result->status |= FF_BATTERY_STATUS_DISCHARGING;
        FF_STRBUF_AUTO_DESTROY now = ffStrbufCreate();
        if (ffReadFileBufferRelative(dfd, "time_to_empty_now", &now)) {
            result->timeRemaining = (int32_t) ffStrbufToSInt(&now, 0);
        } else {
            if (ffReadFileBufferRelative(dfd, "charge_now", &now)) {
                int64_t chargeNow = ffStrbufToSInt(&now, 0);
                if (chargeNow > 0) {
                    if (ffReadFileBufferRelative(dfd, "current_now", &now)) {
                        int64_t currentNow = ffStrbufToSInt(&now, INT64_MIN);
                        if (currentNow < 0) {
                            currentNow = -currentNow;
                        }
                        if (currentNow > 0) {
                            result->timeRemaining = (int32_t) ((chargeNow * 3600) / currentNow);
                        }
                    }
                }
            }
        }
    } else if (ffStrbufEqualS(&tmpBuffer, "Charging")) {
        result->status |= FF_BATTERY_STATUS_CHARGING;
    } else if (ffStrbufEqualS(&tmpBuffer, "Unknown")) {
        result->status |= FF_BATTERY_STATUS_UNKNOWN;
    }

    if (ffReadFileBufferRelative(dfd, "capacity_level", &tmpBuffer)) {
        ffStrbufTrimRightSpace(&tmpBuffer);
        if (ffStrbufEqualS(&tmpBuffer, "Critical")) {
            result->status |= FF_BATTERY_STATUS_CRITICAL;
        }
    }

    if (ffReadFileBufferRelative(dfd, "serial_number", &result->serial)) {
        ffStrbufTrimRightSpace(&result->serial);
    }

    if (ffReadFileBufferRelative(dfd, "cycle_count", &tmpBuffer)) {
        int64_t cycleCount = ffStrbufToSInt(&tmpBuffer, 0);
        result->cycleCount = cycleCount < 0 || cycleCount > UINT32_MAX ? 0 : (uint32_t) cycleCount;
    }

    if (ffReadFileBufferRelative(dfd, "manufacture_year", &tmpBuffer)) {
        int year = (int) ffStrbufToSInt(&tmpBuffer, 0);
        if (year > 0) {
            if (ffReadFileBufferRelative(dfd, "manufacture_month", &tmpBuffer)) {
                int month = (int) ffStrbufToSInt(&tmpBuffer, 0);
                if (month > 0) {
                    if (ffReadFileBufferRelative(dfd, "manufacture_day", &tmpBuffer)) {
                        int day = (int) ffStrbufToSInt(&tmpBuffer, 0);
                        if (day > 0) {
                            ffStrbufSetF(&result->manufactureDate, "%.4d-%.2d-%.2d", year, month, day);
                        }
                    }
                }
            }
        }
    }

    if (options->temp) {
        if (ffReadFileBufferRelative(dfd, "temp", &tmpBuffer)) {
            result->temperature = ffStrbufToDouble(&tmpBuffer, FF_BATTERY_TEMP_UNSET);
            if (result->temperature != FF_BATTERY_TEMP_UNSET) {
                result->temperature /= 10;
            }
        }
    }

    FF_DEBUG("Battery \"%s\": Capacity: %.2f%%, Status: \"%x\", Time Remaining: %d seconds, Temperature: %.1f°C, Cycle Count: %u",
        id,
        result->capacity,
        result->status,
        result->timeRemaining,
        result->temperature,
        result->cycleCount);
    return true;
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results) {
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/power_supply/");
    if (dirp == NULL) {
        return "opendir(\"/sys/class/power_supply/\") == NULL";
    }

    bool acConnected = false;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        FF_AUTO_CLOSE_FD int dfd = openat(dirfd(dirp), entry->d_name, O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
        if (dfd >= 0) {
            parseBattery(dfd, entry->d_name, options, results, &acConnected);
        }
    }

    if (acConnected) {
        FF_LIST_FOR_EACH (FFBatteryResult, batt, *results) {
            batt->status |= FF_BATTERY_STATUS_AC_CONNECTED;
        }
    }

    return NULL;
}
