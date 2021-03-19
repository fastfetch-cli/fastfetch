#include "fastfetch.h"

#include <dirent.h>

static void printBattery(FFinstance* instance, uint8_t index)
{
    char manufactor[128];
    char manufactorPath[42];
    sprintf(manufactorPath, "/sys/class/power_supply/BAT%i/manufacturer", index);
    ffGetFileContent(manufactorPath, manufactor, sizeof(manufactor));

    char model[128];
    char modelPath[40];
    sprintf(modelPath, "/sys/class/power_supply/BAT%i/model_name", index);
    ffGetFileContent(modelPath, model, sizeof(model));

    char technology[32];
    char technologyPath[40];
    sprintf(technologyPath, "/sys/class/power_supply/BAT%i/technology", index);
    ffGetFileContent(technologyPath, technology, sizeof(technology));

    char capacity[32];
    char capacityPath[38];
    sprintf(capacityPath, "/sys/class/power_supply/BAT%i/capacity", index);
    ffGetFileContent(capacityPath, capacity, sizeof(capacity));

    char status[32];
    char statusPath[36];
    sprintf(statusPath, "/sys/class/power_supply/BAT%i/status", index);
    ffGetFileContent(statusPath, status, sizeof(status));

    char key[10];
    sprintf(key, "Battery %i", index);

    if(manufactor[0] == '\0' && model[0] == '\0' && technology[0] == '\0' && capacity[0] == '\0' && status[0] == '\0' && ffStrbufIsEmpty(&instance->config.batteryFormat))
    {
        ffPrintError(instance, key, "No file in /sys/class/power_supply/BAT0/ could be read or all battery options are disabled");
        return;
    }

    ffPrintLogoAndKey(instance, key);

    if(ffStrbufIsEmpty(&instance->config.batteryFormat))
    {
        if(manufactor[0] != '\0')
            printf("%s ", manufactor);

        if(model[0] != '\0')
            printf("%s ", model);

        if(technology[0] != '\0')
            printf("(%s) ", technology);

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
    else
    {
        FFstrbuf battery;
        ffStrbufInit(&battery);

        ffParseFormatString(&battery, &instance->config.batteryFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, manufactor},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, model},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, technology},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, capacity},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, status}
        );

        ffStrbufWriteTo(&battery, stdout);
        putchar('\n');
        ffStrbufDestroy(&battery);
    }
}

void ffPrintBattery(FFinstance* instance)
{
    for(uint8_t i = 0; i < 5; i++)
    {
        char path[30];
        sprintf(path, "/sys/class/power_supply/BAT%i", i);

        DIR* dir = opendir(path);
        if(dir != NULL)
        {
            printBattery(instance, i);
            closedir(dir);
        }
    }
}