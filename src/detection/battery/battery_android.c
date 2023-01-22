#include "fastfetch.h"
#include "battery.h"

#include "common/processing.h"
#include "common/properties.h"

const char* ffDetectBatteryImpl(FF_MAYBE_UNUSED FFinstance* instance, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY buffer;
    ffStrbufInit(&buffer);

    if(ffProcessAppendStdOut(&buffer, (char* const[]){
        FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api",
        "BatteryStatus"
    }))
        return "Starting `" FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api" " BatteryStatus` failed";

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

    if(ffParsePropLines(buffer.chars, "\"status\": ", &battery->status))
    {
        ffStrbufTrimRight(&battery->status, ',');
        ffStrbufTrim(&battery->status, '"');
    }

    return NULL;
}
