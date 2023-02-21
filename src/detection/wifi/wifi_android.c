#include "wifi.h"

#include "common/processing.h"
#include "common/properties.h"

#define FF_TERMUX_API_PATH FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api"
#define FF_TERMUX_API_PARAM "WifiConnectionInfo"

const char* ffDetectWifi(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY buffer;
    ffStrbufInit(&buffer);

    if(ffProcessAppendStdOut(&buffer, (char* const[]){
        FF_TERMUX_API_PATH,
        "WifiConnectionInfo"
    }))
        return "Starting `" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` failed";

    if(buffer.length == 0)
        return "`" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` prints empty";

    FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
    ffStrbufInit(&item->inf.description);
    ffStrbufInit(&item->inf.status);
    ffStrbufInit(&item->conn.status);
    ffStrbufInit(&item->conn.ssid);
    ffStrbufInit(&item->conn.macAddress);
    ffStrbufInit(&item->conn.protocol);
    ffStrbufInit(&item->conn.security);
    item->conn.signalQuality = 0.0/0.0;
    item->conn.rxRate = 0.0/0.0;
    item->conn.txRate = 0.0/0.0;

    if(!ffParsePropLines(buffer.chars, "\"supplicant_state\": ", &item->inf.status))
        ffStrbufAppendS(&item->inf.status, "Unknown");

    {
        ffStrbufTrimRight(&item->inf.status, ',');
        ffStrbufTrim(&item->inf.status, '"');
        if(!ffStrbufEqualS(&item->inf.status, "COMPLETED"))
            return NULL;
    }

    if(ffParsePropLines(buffer.chars, "\"rssi\": ", &item->inf.description))
    {
        double rssi = ffStrbufToDouble(&item->inf.description);
        item->conn.signalQuality = rssi >= -50 ? 100 : rssi <= -100 ? 0 : (rssi + 100) * 2;
        ffStrbufClear(&item->inf.description);
    }

    if(ffParsePropLines(buffer.chars, "\"network_id\": ", &item->inf.description))
        ffStrbufTrimRight(&item->inf.description, ',');

    if(ffParsePropLines(buffer.chars, "\"bssid\": ", &item->conn.macAddress))
    {
        ffStrbufTrimRight(&item->conn.macAddress, ',');
        ffStrbufTrim(&item->conn.macAddress, '"');
    }

    if(ffParsePropLines(buffer.chars, "\"ssid\": ", &item->conn.ssid))
    {
        ffStrbufTrimRight(&item->conn.ssid, ',');
        ffStrbufTrim(&item->conn.ssid, '"');
    }

    return NULL;
}
