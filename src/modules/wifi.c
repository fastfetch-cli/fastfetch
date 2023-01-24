#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wifi/wifi.h"

#define FF_WIFI_MODULE_NAME "Wifi"
#define FF_WIFI_NUM_FORMAT_ARGS 10

void ffPrintWifi(FFinstance* instance)
{
    FF_LIST_AUTO_DESTROY result;
    ffListInit(&result, sizeof(FFWifiResult));

    const char* error = ffDetectWifi(instance, &result);
    if(error)
    {
        ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &instance->config.wifi, "%s", error);
        return;
    }
    if(!result.length)
    {
        ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &instance->config.wifi, "No Wifi interfaces found");
        return;
    }

    for(uint32_t index = 0; index < result.length; ++index)
    {
        FFWifiResult* item = (FFWifiResult*)ffListGet(&result, index);
        uint8_t moduleIndex = result.length == 1 ? 0 : (uint8_t)(index + 1);

        if(instance->config.wifi.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_WIFI_MODULE_NAME, moduleIndex, &instance->config.wifi.key);
            if(item->conn.ssid.length)
            {
                ffStrbufWriteTo(&item->conn.ssid, stdout);
                if(item->conn.protocol.length)
                    printf(" - %s", item->conn.protocol.chars);
                if(item->conn.security.length)
                    printf(" - %s", item->conn.security.chars);
                putchar('\n');
            }
            else
            {
                puts(item->inf.status.chars);
            }
        }
        else
        {
            ffPrintFormat(instance, FF_WIFI_MODULE_NAME, moduleIndex, &instance->config.wifi, FF_WIFI_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->inf.description},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->inf.status},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->conn.status},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->conn.ssid},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->conn.macAddress},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->conn.protocol},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &item->conn.signalQuality},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &item->conn.rxRate},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &item->conn.txRate},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->conn.security},
            });
        }

        ffStrbufDestroy(&item->inf.description);
        ffStrbufDestroy(&item->inf.status);
        ffStrbufDestroy(&item->conn.status);
        ffStrbufDestroy(&item->conn.ssid);
        ffStrbufDestroy(&item->conn.macAddress);
        ffStrbufDestroy(&item->conn.protocol);
        ffStrbufDestroy(&item->conn.security);
    }
}
