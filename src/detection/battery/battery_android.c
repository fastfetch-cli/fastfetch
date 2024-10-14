#include "fastfetch.h"
#include "battery.h"
#include "util/stringUtils.h"
#include "common/processing.h"
#include "common/properties.h"

#define FF_TERMUX_API_PATH FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api"
#define FF_TERMUX_API_PARAM "BatteryStatus"

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

static const char* parseTermuxApi(FFBatteryOptions* options, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if(ffProcessAppendStdOut(&buffer, (char* const[]){
        FF_TERMUX_API_PATH,
        FF_TERMUX_API_PARAM,
        NULL
    }))
        return "Starting `" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` failed";

    yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(buffer.chars, buffer.length, 0, NULL, NULL);
    if (!doc)
        return "Failed to parse battery info";

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_obj(root))
        return "Battery info result is not a JSON object";

    FFBatteryResult* battery = ffListAdd(results);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->status);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    ffStrbufInit(&battery->manufactureDate);

    battery->capacity = yyjson_get_num(yyjson_obj_get(root, "percentage"));
    const char* acStatus = yyjson_get_str(yyjson_obj_get(root, "plugged"));
    if (acStatus)
    {
        if (ffStrEquals(acStatus, "PLUGGED_AC"))
            ffStrbufAppendS(&battery->status, "AC Connected, ");
        else if (ffStrEquals(acStatus, "PLUGGED_USB"))
            ffStrbufAppendS(&battery->status, "USB Connected, ");
        else if (ffStrEquals(acStatus, "PLUGGED_WIRELESS"))
            ffStrbufAppendS(&battery->status, "Wireless Connected, ");
    }
    const char* status = yyjson_get_str(yyjson_obj_get(root, "status"));
    if (status)
    {
        if (ffStrEquals(status, "CHARGING"))
            ffStrbufAppendS(&battery->status, "Charging");
        else if (ffStrEquals(status, "DISCHARGING"))
            ffStrbufAppendS(&battery->status, "Discharging");
    }
    ffStrbufTrimRight(&battery->status, ' ');
    ffStrbufTrimRight(&battery->status, ',');

    if(options->temp)
        battery->temperature = yyjson_get_num(yyjson_obj_get(root, "temperature"));

    return NULL;
}

static const char* parseDumpsys(FFBatteryOptions* options, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buf, (char* []) {
        "/system/bin/dumpsys",
        "battery",
        NULL,
    }) != NULL || buf.length == 0)
        return "Executing `/system/bin/dumpsys battery` failed"; // Only works in `adb shell`, or when rooted

    if (!ffStrbufStartsWithS(&buf, "Current Battery Service state:\n"))
        return "Invalid `/system/bin/dumpsys battery` result";

    const char* start = buf.chars + strlen("Current Battery Service state:\n");

    FF_STRBUF_AUTO_DESTROY temp = ffStrbufCreate();
    if (!ffParsePropLines(start, "present: ", &temp) || !ffStrbufEqualS(&temp, "true"))
        return NULL;
    ffStrbufClear(&temp);

    FFBatteryResult* battery = ffListAdd(results);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->status);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    ffStrbufInit(&battery->manufactureDate);

    if (ffParsePropLines(start, "AC powered: ", &temp) && ffStrbufEqualS(&temp, "true"))
        ffStrbufAppendS(&battery->status, "AC powered");
    ffStrbufClear(&temp);

    if (ffParsePropLines(start, "USB powered: ", &temp) && ffStrbufEqualS(&temp, "true"))
    {
        if (battery->status.length) ffStrbufAppendS(&battery->status, ", ");
        ffStrbufAppendS(&battery->status, "USB powered");
    }
    ffStrbufClear(&temp);

    if (ffParsePropLines(start, "Wireless powered: ", &temp) && ffStrbufEqualS(&temp, "true"))
    {
        if (battery->status.length) ffStrbufAppendS(&battery->status, ", ");
        ffStrbufAppendS(&battery->status, "Wireless powered");
    }
    ffStrbufClear(&temp);

    {
        double level = 0, scale = 0;
        if (ffParsePropLines(start, "level: ", &temp))
            level = ffStrbufToDouble(&temp);
        ffStrbufClear(&temp);

        if (ffParsePropLines(start, "scale: ", &temp))
            scale = ffStrbufToDouble(&temp);
        ffStrbufClear(&temp);

        if (level > 0 && scale > 0)
            battery->capacity = level * 100 / scale;
    }

    if(options->temp)
    {
        if (ffParsePropLines(start, "temperature: ", &temp))
            battery->temperature = ffStrbufToDouble(&temp);
        ffStrbufClear(&temp);
    }

    ffParsePropLines(start, "technology: ", &battery->technology);

    return NULL;
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    const char* error = parseTermuxApi(options, results);
    if (error && parseDumpsys(options, results) == NULL)
        return NULL;
    return error;
}
