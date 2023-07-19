#include "fastfetch.h"
#include "battery.h"

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

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
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

    BatteryResult* battery = ffListAdd(results);
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->status);
    ffStrbufInit(&battery->technology);

    battery->capacity = yyjson_get_num(yyjson_obj_get(root, "percentage"));
    ffStrbufAppendS(&battery->status, yyjson_get_str(yyjson_obj_get(root, "status")));
    if(options->temp)
        battery->temperature = yyjson_get_num(yyjson_obj_get(root, "temperature"));

    return NULL;
}
