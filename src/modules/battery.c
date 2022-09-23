#include "fastfetch.h"
#include "common/printing.h"
#include "detection/battery/battery.h"

#define FF_BATTERY_MODULE_NAME "Battery"
#define FF_BATTERY_NUM_FORMAT_ARGS 5

static void printBattery(FFinstance* instance, const BatteryResult* result, uint8_t index)
{
    if(instance->config.battery.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.battery.key);

        bool showStatus =
            result->status.length > 0 &&
            ffStrbufIgnCaseCompS(&result->status, "Unknown") != 0;

        if(result->capacity.length > 0)
        {
            ffStrbufWriteTo(&result->capacity, stdout);
            putchar('%');

            if(showStatus)
                fputs(" [", stdout);
        }

        if(showStatus)
        {
            ffStrbufWriteTo(&result->status, stdout);

            if(result->capacity.length > 0)
                putchar(']');
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.battery, FF_BATTERY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufacturer},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->modelName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->technology},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->capacity},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->status}
        });
    }
}

void ffPrintBattery(FFinstance* instance)
{
    FFlist results;
    ffListInitA(&results, sizeof(BatteryResult), 0);

    const char* error = ffDetectBatteryImpl(instance, &results);

    if (error)
    {
        ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.battery, "%s", error);
    }
    else
    {
        for(uint8_t i = 0; i < (uint8_t) results.length; i++)
        {
            BatteryResult* result = ffListGet(&results, i);
            printBattery(instance, result, i);

            ffStrbufDestroy(&result->manufacturer);
            ffStrbufDestroy(&result->modelName);
            ffStrbufDestroy(&result->technology);
            ffStrbufDestroy(&result->capacity);
            ffStrbufDestroy(&result->status);
        }
        if(results.length == 0)
            ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.battery, "No batteries found");
    }

    ffListDestroy(&results);
}
