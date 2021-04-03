#include "fastfetch.h"

#include <dirent.h>

static void getKey(FFinstance* instance, FFstrbuf* key, uint8_t counter, bool showCounter)
{
    if(instance->config.batteryKey.length == 0)
    {
        if(showCounter)
            ffStrbufAppendF(key, "Battery %hhu", counter);
        else
            ffStrbufSetS(key, "Battery");
    }
    else
    {
        ffParseFormatString(key, &instance->config.batteryKey, 1,
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT8, &counter}
        );
    }
}

static void printBattery(FFinstance* instance, uint8_t index)
{
    FF_STRBUF_CREATE(manufactor);
    char manufactorPath[42];
    sprintf(manufactorPath, "/sys/class/power_supply/BAT%hhu/manufacturer", index);
    ffGetFileContent(manufactorPath, &manufactor);

    FF_STRBUF_CREATE(model);
    char modelPath[40];
    sprintf(modelPath, "/sys/class/power_supply/BAT%hhu/model_name", index);
    ffGetFileContent(modelPath, &model);

    FF_STRBUF_CREATE(technology);
    char technologyPath[40];
    sprintf(technologyPath, "/sys/class/power_supply/BAT%hhu/technology", index);
    ffGetFileContent(technologyPath, &technology);

    FF_STRBUF_CREATE(capacity);
    char capacityPath[38];
    sprintf(capacityPath, "/sys/class/power_supply/BAT%hhu/capacity", index);
    ffGetFileContent(capacityPath, &capacity);

    FF_STRBUF_CREATE(status);
    char statusPath[36];
    sprintf(statusPath, "/sys/class/power_supply/BAT%hhu/status", index);
    ffGetFileContent(statusPath, &status);

    FF_STRBUF_CREATE(key);
    getKey(instance, &key, index + 1, true);

    if(
        manufactor.length == 0 &&
        model.length      == 0 &&
        technology.length == 0 &&
        capacity.length   == 0 &&
        status.length     == 0
    ) {
        ffPrintError(instance, &key, NULL, "No file in /sys/class/power_supply/BAT%hhu/ could be read or all battery options are disabled", index);
        ffStrbufDestroy(&key);
        return;
    }

    if(instance->config.batteryFormat.length == 0)
    {

        ffPrintLogoAndKey(instance, &key, NULL);

        if(manufactor.length > 0)
            printf("%s ", manufactor.chars);

        if(model.length > 0)
            printf("%s ", model.chars);

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
        ffPrintFormatString(instance, &key, NULL, &instance->config.batteryFormat, 5,
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
    ffStrbufDestroy(&key);
}

void ffPrintBattery(FFinstance* instance)
{
    uint8_t batteryCounter = 0;
    for(uint8_t i = 0; i < 5; i++)
    {
        char path[30];
        sprintf(path, "/sys/class/power_supply/BAT%i", i);

        DIR* dir = opendir(path);
        if(dir != NULL)
        {
            printBattery(instance, batteryCounter++);
            closedir(dir);
        }
    }

    if(batteryCounter == 0)
    {
        if(!instance->config.showErrors)
            return;

        FF_STRBUF_CREATE(key);
        getKey(instance, &key, 1, false);
        ffPrintError(instance, &key, NULL, "No battery found in /sys/class/power_supply/");
        ffStrbufDestroy(&key);
    }
}
