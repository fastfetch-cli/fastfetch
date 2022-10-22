#include "fastfetch.h"
#include "common/printing.h"
#include "common/bar.h"
#include "detection/battery/battery.h"

#define FF_BATTERY_MODULE_NAME "Battery"
#define FF_BATTERY_NUM_FORMAT_ARGS 5

static void printBattery(FFinstance* instance, BatteryResult* result, uint8_t index)
{
    if(instance->config.battery.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.battery.key);

        bool showStatus =
            result->status.length > 0 &&
            ffStrbufIgnCaseCompS(&result->status, "Unknown") != 0;

        FFstrbuf str;
        ffStrbufInit(&str);

        if(result->capacity >= 0)
        {
            if(instance->config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                if(result->capacity <= 20)
                    ffAppendPercentBar(instance, &str, (uint8_t)result->capacity, 10, 10, 0);
                else if(result->capacity <= 50)
                    ffAppendPercentBar(instance, &str, (uint8_t)result->capacity, 10, 0, 10);
                else
                    ffAppendPercentBar(instance, &str, (uint8_t)result->capacity, 0, 10, 10);
            }

            if(instance->config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');
                ffStrbufAppendF(&str, "%.0f%%", result->capacity);
            }
        }

        if(showStatus)
        {
            if(str.length > 0)
                ffStrbufAppendF(&str, " [%s]", result->status.chars);
            else
                ffStrbufAppend(&str, &result->status);
        }

        if(result->temperature == result->temperature) //FF_BATTERY_TEMP_UNSET
        {
            if(str.length > 0)
                ffStrbufAppendS(&str, " - ");

            ffStrbufAppendF(&str, "%.1f°C", result->temperature);
        }

        ffStrbufPutTo(&str, stdout);
        ffStrbufDestroy(&str);
    }
    else
    {
        ffPrintFormat(instance, FF_BATTERY_MODULE_NAME, index, &instance->config.battery, FF_BATTERY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufacturer},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->modelName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->technology},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &result->capacity},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->status},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &result->temperature},
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
            ffStrbufDestroy(&result->status);
        }
        if(results.length == 0)
            ffPrintError(instance, FF_BATTERY_MODULE_NAME, 0, &instance->config.battery, "No batteries found");
    }

    ffListDestroy(&results);
}
