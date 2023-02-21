#include "fastfetch.h"
#include "battery.h"

#include "common/processing.h"
#include "common/properties.h"

#define FF_TERMUX_API_PATH FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api"
#define FF_TERMUX_API_PARAM "BatteryStatus"

const char* ffDetectBatteryImpl(FF_MAYBE_UNUSED FFinstance* instance, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY buffer;
    ffStrbufInit(&buffer);

    if(ffProcessAppendStdOut(&buffer, (char* const[]){
        FF_TERMUX_API_PATH,
        FF_TERMUX_API_PARAM
    }))
        return "Starting `" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` failed";

    if(buffer.length == 0)
        return "`" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` prints empty";

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

    if(instance->config.batteryTemp)
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
