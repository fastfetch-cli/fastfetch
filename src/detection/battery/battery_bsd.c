#include "fastfetch.h"
#include "common/sysctl.h"
#include "common/io/io.h"
#include "battery.h"

#include <dev/acpica/acpiio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

const char* ffDetectBattery(FF_MAYBE_UNUSED FFBatteryOptions* options, FFlist* results)
{
    //https://www.freebsd.org/cgi/man.cgi?acpi_battery(4)
    //https://gitlab.xfce.org/panel-plugins/xfce4-battery-plugin/-/blob/master/panel-plugin/libacpi.c

    int units = ffSysctlGetInt("hw.acpi.battery.units", -100);
    if (units < 0)
        return "sysctlbyname(\"hw.acpi.battery.units\") failed";

    if(units == 0)
        return NULL;

    FF_AUTO_CLOSE_FD int acpifd = open("/dev/acpi", O_RDONLY | O_CLOEXEC);
    if(acpifd < 0)
        return "open(\"/dev/acpi\", O_RDONLY | O_CLOEXEC) failed";

    for(int i = 0; i < units; ++i)
    {
        union acpi_battery_ioctl_arg battio;
        battio.unit = i;

        if(ioctl(acpifd, ACPIIO_BATT_GET_BATTINFO, &battio) < 0 || (battio.battinfo.state == ACPI_BATT_STAT_NOT_PRESENT))
            continue;

        FFBatteryResult* battery = ffListAdd(results);
        battery->temperature = FF_BATTERY_TEMP_UNSET;
        battery->cycleCount = 0;
        ffStrbufInit(&battery->manufacturer);
        ffStrbufInit(&battery->modelName);
        ffStrbufInit(&battery->status);
        ffStrbufInit(&battery->technology);
        ffStrbufInit(&battery->serial);
        ffStrbufInit(&battery->manufactureDate);
        battery->timeRemaining = -1;
        if (battio.battinfo.min > 0)
            battery->timeRemaining = battio.battinfo.min * 60;
        battery->capacity = battio.battinfo.cap;
        if(battio.battinfo.state == ACPI_BATT_STAT_INVALID)
        {
            ffStrbufAppendS(&battery->status, "Unknown, ");
        }
        else
        {
            if(battio.battinfo.state & ACPI_BATT_STAT_DISCHARG)
                ffStrbufAppendS(&battery->status, "Discharging, ");
            else if(battio.battinfo.state & ACPI_BATT_STAT_CHARGING)
                ffStrbufAppendS(&battery->status, "Charging, ");
            if(battio.battinfo.state & ACPI_BATT_STAT_CRITICAL)
                ffStrbufAppendS(&battery->status, "Critical, ");
        }

        int acadStatus;
        if (ioctl(acpifd, ACPIIO_ACAD_GET_STATUS, &acadStatus) >= 0 && acadStatus)
        {
            ffStrbufAppendS(&battery->status, "AC Connected");
        }
        else
        {
            ffStrbufTrimRight(&battery->status, ' ');
            ffStrbufTrimRight(&battery->status, ',');
        }

        battio.unit = i;
        if (ioctl(acpifd, ACPIIO_BATT_GET_BIX, &battio) >= 0)
        {
            ffStrbufAppendS(&battery->manufacturer, battio.bix.oeminfo);
            ffStrbufAppendS(&battery->modelName, battio.bix.model);
            ffStrbufAppendS(&battery->technology, battio.bix.type);
            battery->cycleCount = battio.bix.cycles;
        }
    }
    return NULL;
}
