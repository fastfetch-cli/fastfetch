#include "fastfetch.h"

void ffPrintBattery(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "Battery"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    char model[256];
    ffGetFileContent("/sys/class/power_supply/BAT0/model_name", model, sizeof(model));
    
    char technology[32];
    ffGetFileContent("/sys/class/power_supply/BAT0/technology", technology, sizeof(technology));

    char capacity[32];
    ffGetFileContent("/sys/class/power_supply/BAT0/capacity", capacity, sizeof(capacity));

    char status[32];
    ffGetFileContent("/sys/class/power_supply/BAT0/status", status, sizeof(status));

    if(model[0] == '\0' && capacity[0] == '\0' && status[0] == '\0')
    {
        ffPrintError(instance, "Battery", "No file in /sys/class/power_supply/BAT0/ could be read");
        return;
    }

    ffPrintLogoAndKey(instance, "Battery");

    if(model[0] != '\0')
    {
        printf("%s ", model);
        
        if(technology[0] != '\0')
            printf("(%s) ", technology);
    }

    if(capacity[0] != '\0')
    {
        printf("[%s%%", capacity);
    
        if(status[0] == '\0')
            puts("]");
        else
            printf("; %s]\n", status);
    }
    else
    {
        if(status[0] != '\0')
            printf("%s", status);
        else
            putchar('\n');
    }

}