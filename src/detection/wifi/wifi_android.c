#include "wifi.h"

#include "common/processing.h"
#include "common/properties.h"

#define FF_TERMUX_API_PATH FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api"
#define FF_TERMUX_API_PARAM "WifiConnectionInfo"

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

const char* ffDetectWifi(FFlist* result)
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
        return "Failed to parse wifi connection info";

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_obj(root))
        return "Wifi info result is not a JSON object";

    FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
    ffStrbufInit(&item->inf.description);
    ffStrbufInit(&item->inf.status);
    ffStrbufInit(&item->conn.status);
    ffStrbufInit(&item->conn.ssid);
    ffStrbufInit(&item->conn.bssid);
    ffStrbufInit(&item->conn.protocol);
    ffStrbufInit(&item->conn.security);
    item->conn.signalQuality = 0.0/0.0;
    item->conn.rxRate = 0.0/0.0;
    item->conn.txRate = 0.0/0.0;

    ffStrbufAppendS(&item->inf.status, yyjson_get_str(yyjson_obj_get(root, "supplicant_state")));
    if(!item->inf.status.length)
    {
        ffStrbufAppendS(&item->inf.status, "Unknown");
        return NULL;
    }

    if(!ffStrbufEqualS(&item->inf.status, "COMPLETED"))
        return NULL;

    double rssi = yyjson_get_num(yyjson_obj_get(root, "rssi"));
    item->conn.signalQuality = rssi >= -50 ? 100 : rssi <= -100 ? 0 : (rssi + 100) * 2;

    ffStrbufAppendS(&item->inf.description, yyjson_get_str(yyjson_obj_get(root, "ip")));
    ffStrbufAppendS(&item->conn.bssid, yyjson_get_str(yyjson_obj_get(root, "bssid")));
    ffStrbufAppendS(&item->conn.ssid, yyjson_get_str(yyjson_obj_get(root, "ssid")));

    return NULL;
}
