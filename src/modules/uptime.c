#include "fastfetch.h"

void ffPrintUptime(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "Uptime"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    ffPrintLogoAndKey(instance, "Uptime");

    if(instance->state.sysinfo.uptime < 60)
    {
        printf("%ld seconds\n", instance->state.sysinfo.uptime);
        return;
    }

    int days = instance->state.sysinfo.uptime / 86400;
	int hours = (instance->state.sysinfo.uptime - (days * 86400)) / 3600; 
	int minutes = (instance->state.sysinfo.uptime - (days * 86400) - (hours * 3600)) / 60;
	
    if(days > 0)
        printf("%d day%s, ", days, days <= 1 ? "" : "s");

    if(hours > 0)
        printf("%d hour%s, ", hours, hours <= 1 ? "" : "s");

    if(minutes > 0)
        printf("%d min%s", minutes, minutes <= 1 ? "" : "s");

    putchar('\n');
}