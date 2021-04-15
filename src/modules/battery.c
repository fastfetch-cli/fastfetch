#include "fastfetch.h"

#include <dirent.h>

#define FF_BATTERY_MODULE_NAME "Battery"
#define FF_BATTERY_NUM_FORMAT_ARGS 5

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

    if(manufactor.length == 0 && model.length == 0 && technology.length == 0 && capacity.length == 0 && status.length == 0)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "No file in /sys/class/power_supply/BAT%hhu/ could be read or all battery options are disabled", index);
        return;
    }

    if(instance->config.batteryFormat.length == 0)
    {

        ffPrintLogoAndKey(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey);

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
        ffPrintFormatString(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.batteryKey, &instance->config.batteryFormat, NULL, FF_BATTERY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &manufactor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &model},
            {FF_FORMAT_ARG_TYPE_STRBUF, &technology},
            {FF_FORMAT_ARG_TYPE_STRBUF, &capacity},
            {FF_FORMAT_ARG_TYPE_STRBUF, &status}
        });
    }

    ffStrbufDestroy(&manufactor);
    ffStrbufDestroy(&model);
    ffStrbufDestroy(&technology);
    ffStrbufDestroy(&capacity);
    ffStrbufDestroy(&status);
}

#define FF_BATTERY_MAX_INDEX 5

void ffPrintBattery(FFinstance* instance)
{
    uint8_t batteryCounter = 0;
    uint8_t batteryIndicies[FF_BATTERY_MAX_INDEX];

    for(uint8_t i = 0; i < FF_BATTERY_MAX_INDEX; i++)
    {
        char path[30];
        sprintf(path, "/sys/class/power_supply/BAT%i", i);

        DIR* dir = opendir(path);
        if(dir != NULL)
        {
            batteryIndicies[batteryCounter++] = i;
            closedir(dir);
        }
    }

    if(batteryCounter == 0)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.batteryKey, &instance->config.batteryFormat, FF_BATTERY_NUM_FORMAT_ARGS, "/sys/class/power_supply/ doesn't contain any BAT* folder");
        return;
    }

    for(uint8_t i = 0; i < batteryCounter; i++)
    {
        printBattery(instance, batteryIndicies[i]);
    }
}
