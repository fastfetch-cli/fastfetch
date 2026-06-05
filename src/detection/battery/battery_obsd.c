#include "battery.h"
#include "common/io.h"

#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

const char* ffDetectBattery(FF_A_UNUSED FFBatteryOptions* options, FFlist* result) {
    FF_AUTO_CLOSE_FD int devfd = open("/dev/apm", O_RDONLY | O_CLOEXEC);

    if (devfd < 0) {
        return "open(dev/apm, O_RDONLY | O_CLOEXEC) failed";
    }

    struct apm_power_info info = {};

    if (ioctl(devfd, APM_IOC_GETPOWER, &info) < 0) {
        return "ioctl(APM_IOC_GETPOWER) failed";
    }

    if (info.battery_state == APM_BATTERY_ABSENT) {
        return NULL;
    }

    FFBatteryResult* battery = FF_LIST_ADD(FFBatteryResult, *result);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;
    battery->capacity = info.battery_life;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    ffStrbufInit(&battery->manufactureDate);
    battery->status = FF_BATTERY_STATUS_NONE;

    if (info.ac_state == APM_AC_ON || info.ac_state == APM_AC_BACKUP) {
        battery->status |= FF_BATTERY_STATUS_AC_CONNECTED;
    } else if (info.ac_state == APM_AC_OFF) {
        battery->timeRemaining = (int) info.minutes_left * 60;
        battery->status |= FF_BATTERY_STATUS_DISCHARGING;
    }

    if (info.battery_state == APM_BATT_CRITICAL || info.battery_state == APM_BATT_CHARGING || info.battery_state == APM_BATT_UNKNOWN) {
        switch (info.battery_state) {
            case APM_BATT_UNKNOWN:
                battery->status |= FF_BATTERY_STATUS_UNKNOWN;
                break;
            case APM_BATT_CHARGING:
                battery->status |= FF_BATTERY_STATUS_CHARGING;
                break;
            case APM_BATT_CRITICAL:
                battery->status |= FF_BATTERY_STATUS_CRITICAL;
                break;
        }
    }

    return NULL;
}
