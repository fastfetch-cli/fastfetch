#include "fastfetch.h"

#include <dirent.h>

static void printBattery(FFinstance* instance, uint8_t index)
{
    FF_STRBUF_CREATE(manufactor);
    char manufactorPath[42];
    sprintf(manufactorPath, "/sys/class/power_supply/BAT%i/manufacturer", index);
    ffGetFileContent(manufactorPath, &manufactor);

    FF_STRBUF_CREATE(model);
    char modelPath[40];
    sprintf(modelPath, "/sys/class/power_supply/BAT%i/model_name", index);
    ffGetFileContent(modelPath, &model);

    FF_STRBUF_CREATE(technology);
    char technologyPath[40];
    sprintf(technologyPath, "/sys/class/power_supply/BAT%i/technology", index);
    ffGetFileContent(technologyPath, &technology);

    FF_STRBUF_CREATE(capacity);
    char capacityPath[38];
    sprintf(capacityPath, "/sys/class/power_supply/BAT%i/capacity", index);
    ffGetFileContent(capacityPath, &capacity);

    FF_STRBUF_CREATE(status);
    char statusPath[36];
    sprintf(statusPath, "/sys/class/power_supply/BAT%i/status", index);
    ffGetFileContent(statusPath, &status);

    char key[10];
    sprintf(key, "Battery %i", index);

    if(instance->config.batteryFormat.length == 0)
    {
        if(
            manufactor.length == 0 &&
            model.length      == 0 &&
            technology.length == 0 &&
            capacity.length   == 0 &&
            status.length     == 0
        ) {
            ffPrintError(instance, key, "No file in /sys/class/power_supply/BAT0/ could be read or all battery options are disabled");
            return;
        }

        ffPrintLogoAndKey(instance, key);

        if(manufactor.length > 0)
            printf("%s ", manufactor.chars);

        if(model.length > 0)
            printf("%s ", manufactor.chars);

        if(technology.length > 0)
            printf("(%s) ", technology.chars);

        if(capacity.length > 0)
        {
            printf("[%s%%", capacity.chars);

            if(status.length == 0)
                puts("]");
            else
                printf("; %s]\n", status.chars);
        }
        else
        {
            if(status.length > 0)
                printf("[%s]", status.chars);
            else
                putchar('\n');
        }
    }
    else
    {
        ffPrintFormatString(instance, key, &instance->config.batteryFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &manufactor},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &model},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &technology},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &capacity},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &status}
        );
    }

    ffStrbufDestroy(&manufactor);
    ffStrbufDestroy(&model);
    ffStrbufDestroy(&technology);
    ffStrbufDestroy(&capacity);
    ffStrbufDestroy(&status);
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
