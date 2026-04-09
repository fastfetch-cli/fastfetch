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

    FFBatteryResult* battery = (FFBatteryResult*) ffListAdd(result);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;
    battery->capacity = info.battery_life;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->status);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    ffStrbufInit(&battery->manufactureDate);

    if (info.ac_state == APM_AC_ON) {
        ffStrbufAppendS(&battery->status, "AC Connected");
    } else if (info.ac_state == APM_AC_BACKUP) {
        ffStrbufAppendS(&battery->status, "Backup In Use");
    } else if (info.ac_state == APM_AC_OFF) {
        battery->timeRemaining = (int) info.minutes_left * 60;
        ffStrbufAppendS(&battery->status, "Discharging");
    }

    if (info.battery_state == APM_BATT_CRITICAL || info.battery_state == APM_BATT_CHARGING || info.battery_state == APM_BATT_UNKNOWN) {
        if (battery->status.length) {
            ffStrbufAppendS(&battery->status, ", ");
        }
        switch (info.battery_state) {
            case APM_BATT_UNKNOWN:
                ffStrbufAppendS(&battery->status, "Unknown");
                break;
            case APM_BATT_CHARGING:
                ffStrbufAppendS(&battery->status, "Charging");
                break;
            case APM_BATT_CRITICAL:
                ffStrbufAppendS(&battery->status, "Critical");
                break;
        }
    }

    return NULL;
}
