#include "fastfetch.h"

void ffPrintUptime(FFstate* state)
{

    if(state->sysinfo.uptime < 60)
    {
        printf("%ls seconds\n", state->sysinfo.uptime);
        return;
    }

    int days = state->sysinfo.uptime / 86400;
	int hours = (state->sysinfo.uptime - (days * 86400)) / 3600; 
	int minutes = (state->sysinfo.uptime - (days * 86400) - (hours * 3600)) / 60;
	
    ffPrintLogoAndKey(state, "Uptime");

    if(days > 0)
        printf("%d day%s, ", days, days <= 1 ? "" : "s");

    if(hours > 0)
        printf("%d hour%s, ", hours, hours <= 1 ? "" : "s");

    if(minutes > 0)
        printf("%d min%s", minutes, minutes <= 1 ? "" : "s");

    putchar('\n');
}