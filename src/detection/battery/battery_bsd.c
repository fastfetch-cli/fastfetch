#include "fastfetch.h"
#include "battery.h"

#include <dev/acpica/acpiio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

const char* ffDetectBatteryImpl(FF_UNUSED_PARAM FFinstance* instance, FFlist* results)
{
    //https://www.freebsd.org/cgi/man.cgi?acpi_battery(4)
    //https://gitlab.xfce.org/panel-plugins/xfce4-battery-plugin/-/blob/master/panel-plugin/libacpi.c

    int acpifd = open("/dev/acpi", O_RDONLY);
    if(acpifd < 0)
        return "open(\"/dev/acpi\", O_RDONLY) failed";

    int units = ioctl(acpifd, ACPIIO_BATT_GET_UNITS, 0);
    if(units < 0)
    {
        close(acpifd);
        return "No acpiio battery units found";
    }

    for(int i = 0; i < units; ++i)
    {
        union acpi_battery_ioctl_arg battio;
        battio.unit = i;

        if(ioctl(acpifd, ACPIIO_BATT_GET_BATTINFO, &battio) < 0 || (battio.battinfo.state & ACPI_BATT_STAT_NOT_PRESENT))
            continue;

        BatteryResult* battery = ffListAdd(results);
        battery->temperature = FF_BATTERY_TEMP_UNSET;
        ffStrbufInit(&battery->manufacturer);
        ffStrbufInit(&battery->modelName);
        ffStrbufInit(&battery->status);
        ffStrbufInit(&battery->technology);

        battery->capacity = battio.battinfo.cap;
        if(battio.battinfo.state & ACPI_BATT_STAT_INVALID)
        {
            ffStrbufAppendS(&battery->status, "Invalid");
        }
        else
        {
            if(battio.battinfo.state & ACPI_BATT_STAT_DISCHARG)
                ffStrbufAppendS(&battery->status, "Discharging, ");
            else if(battio.battinfo.state & ACPI_BATT_STAT_CHARGING)
                ffStrbufAppendS(&battery->status, "Charging, ");
            if(battio.battinfo.state & ACPI_BATT_STAT_CRITICAL)
                ffStrbufAppendS(&battery->status, "Ctritical");
            ffStrbufTrimRight(&battery->status, ' ');
            ffStrbufTrimRight(&battery->status, ',');
        }
    }
    close(acpifd);
    return NULL;
}
