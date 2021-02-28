#include "fastfetch.h"

void ffPrintBattery(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "Battery"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    int scanned;

    FILE* fullFile = fopen("/sys/class/power_supply/BAT0/charge_full", "r");
    if(fullFile == NULL)
    {
        ffPrintError(instance, "Battery", "fopen(\"/sys/class/power_supply/BAT0/charge_full\", \"r\") == NULL");
        return;
    }
    uint32_t full;
    scanned = fscanf(fullFile, "%u", &full);
    if(scanned != 1)
    {
        ffPrintError(instance, "Battery", "fscanf(fullFile, \"%u\", &full) != 1");
        return;
    }
    fclose(fullFile);

    FILE* nowFile = fopen("/sys/class/power_supply/BAT0/charge_now", "r");
    if(nowFile == NULL)
    {
        ffPrintError(instance, "Battery", "fopen(\"/sys/class/power_supply/BAT0/charge_now\", \"r\") == NULL");
        return;
    }
    uint32_t now;
    scanned = fscanf(nowFile, "%u", &now);
    if(scanned != 1)
    {
        ffPrintError(instance, "Battery", "fscanf(nowFile, \"%u\", &now) != 1");
        return;
    }
    fclose(nowFile);

    FILE* statusFile = fopen("/sys/class/power_supply/BAT0/status", "r");
    if(statusFile == NULL)
    {
        ffPrintError(instance, "Battery", "fopen(\"/sys/class/power_supply/BAT0/status\", \"r\") == NULL");
        return;
    }
    char status[256];
    scanned = fscanf(statusFile, "%s", status);
    if(scanned != 1)
    {
        ffPrintError(instance, "Battery", "fscanf(statusFile, \"%u\", &status) != 1");
        return;
    }
    fclose(statusFile);

    uint32_t percentage = (now / (double) full) * 100;

    ffPrintLogoAndKey(instance, "Battery");
    printf("%u%% [%s]\n", percentage, status);
}