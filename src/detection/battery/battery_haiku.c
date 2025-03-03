#include "fastfetch.h"
#include "battery.h"
#include "common/io/io.h"

#include <private/device/power_managment.h>
#include <sys/ioctl.h>
#include <fcntl.h>

const char* parseBattery(int dfd, const char* battId, FFlist* results)
{
    FF_AUTO_CLOSE_FD int fd = openat(dfd, battId, O_RDWR);
    if (fd < 0) return "openat() failed";

    acpi_battery_info basic = {};
    if (ioctl(fd, GET_BATTERY_INFO, &basic, sizeof(basic)) != 0)
        return "ioctl(GET_BATTERY_INFO) failed";
    acpi_extended_battery_info extended = {};
    if (ioctl(fd, GET_EXTENDED_BATTERY_INFO, &extended, sizeof(extended)) != 0)
        return "ioctl(GET_EXTENDED_BATTERY_INFO) failed";

    if (extended.last_full_charge == (uint32)-1)
        return "Skipped";

    FFBatteryResult* battery = (FFBatteryResult*)ffListAdd(results);
    ffStrbufInitS(&battery->modelName, extended.model_number);
    ffStrbufInitS(&battery->manufacturer, extended.oem_info);
    ffStrbufInit(&battery->manufactureDate);
    ffStrbufInitS(&battery->technology, extended.type); // extended.technology?
    ffStrbufInit(&battery->status);
    ffStrbufInitS(&battery->serial, extended.serial_number);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = extended.cycles;
    battery->timeRemaining = -1;
    battery->capacity = (double) basic.capacity * 100. / (double) extended.last_full_charge;

    if (basic.state & BATTERY_DISCHARGING)
        ffStrbufAppendS(&battery->status, "Discharging, ");
    if (basic.state & BATTERY_CHARGING)
        ffStrbufAppendS(&battery->status, "Charging, ");
    if (basic.state & BATTERY_CRITICAL_STATE)
        ffStrbufAppendS(&battery->status, "Critical, ");
    if (basic.state & BATTERY_NOT_CHARGING)
        ffStrbufAppendS(&battery->status, "AC Connected, ");
    ffStrbufTrimRight(&battery->status, ' ');
    ffStrbufTrimRight(&battery->status, ',');

    return NULL;
}

const char* ffDetectBattery(FF_MAYBE_UNUSED FFBatteryOptions* options, FFlist* results)
{
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/dev/power/acpi_battery/");
    if (!dir) return "opendir(/dev/power/acpi_battery) failed";

    struct dirent* entry;
    while ((entry = readdir(dir)))
    {
        if (entry->d_name[0] == '.') continue;
        parseBattery(dirfd(dir), entry->d_name, results);
    }

    return NULL;
}
