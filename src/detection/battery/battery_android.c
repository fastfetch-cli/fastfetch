#include "fastfetch.h"
#include "battery.h"

#include "common/processing.h"
#include "common/properties.h"

#define FF_TERMUX_API_PATH FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api"
#define FF_TERMUX_API_PARAM "BatteryStatus"

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if(ffProcessAppendStdOut(&buffer, (char* const[]){
        FF_TERMUX_API_PATH,
        FF_TERMUX_API_PARAM,
        NULL
    }))
        return "Starting `" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` failed";

    if(buffer.chars[0] != '{')
        return "`" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` prints invalid result (not a JSON object)";

    BatteryResult* battery = ffListAdd(results);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->status);
    ffStrbufInit(&battery->technology);

    if(ffParsePropLines(buffer.chars, "\"percentage\": ", &battery->status))
    {
        battery->capacity = ffStrbufToDouble(&battery->status);
        ffStrbufClear(&battery->status);
    }

    if(instance->config.battery.temp)
    {
        if(ffParsePropLines(buffer.chars, "\"temperature\": ", &battery->status))
        {
            ffStrbufTrimRight(&battery->status, ',');
            ffStrbufTrim(&battery->status, '"');
            battery->temperature = ffStrbufToDouble(&battery->status);
            ffStrbufClear(&battery->status);
        }
    }

    if(ffParsePropLines(buffer.chars, "\"status\": ", &battery->status))
    {
        ffStrbufTrimRight(&battery->status, ',');
        ffStrbufTrim(&battery->status, '"');
    }

    return NULL;
}
