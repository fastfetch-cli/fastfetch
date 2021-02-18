#include "fastfetch.h"

void ffPrintBattery(FFstate* state)
{

    FILE* fullFile = fopen("/sys/class/power_supply/BAT0/charge_full", "r");
    if(fullFile == NULL)
        return;
    uint32_t full;
    fscanf(fullFile, "%lu", &full);
    fclose(fullFile);

    FILE* nowFile = fopen("/sys/class/power_supply/BAT0/charge_full", "r");
    if(nowFile == NULL)
        return;
    uint32_t now;
    fscanf(nowFile, "%lu", &now);
    fclose(nowFile);

    FILE* statusFile = fopen("/sys/class/power_supply/BAT0/status", "r");
    if(statusFile == NULL)
        return;
    char status[256];
    fscanf(statusFile, "%s", status);
    fclose(statusFile);

    uint32_t percentage = (now / (double) full) * 100;

    ffPrintLogoAndKey(state, "Battery");
    printf("%lu%% [%s]\n", percentage, status);
}