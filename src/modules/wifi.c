#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wifi/wifi.h"

#define FF_WIFI_MODULE_NAME "Wifi"
#define FF_WIFI_NUM_FORMAT_ARGS 13

void ffPrintWifi(FFinstance* instance)
{
    FFlist result;
    ffListInit(&result, sizeof(FFWifiResult));

    const char* error = ffDetectWifi(instance, &result);

    if(!error)
    {
        for(uint32_t index = 0; index < result.length; ++index)
        {
            FFWifiResult* item = (FFWifiResult*)ffListGet(&result, index);
            uint8_t moduleIndex = result.length == 1 ? 0 : (uint8_t)(index + 1);

            if(instance->config.wifi.outputFormat.length == 0)
            {
                ffPrintLogoAndKey(instance, FF_WIFI_MODULE_NAME, moduleIndex, &instance->config.wifi.key);
                if(item->conn.ssid.length)
                {
                    printf("%s - %s", item->conn.ssid.chars, item->conn.phyType.chars);
                    if(!item->security.enabled)
                        puts(" - insecure");
                    else
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
                    {FF_FORMAT_ARG_TYPE_STRBUF, &item->conn.phyType},
                    {FF_FORMAT_ARG_TYPE_DOUBLE, &item->conn.signalQuality},
                    {FF_FORMAT_ARG_TYPE_DOUBLE, &item->conn.rxRate},
                    {FF_FORMAT_ARG_TYPE_DOUBLE, &item->conn.txRate},
                    {FF_FORMAT_ARG_TYPE_BOOL, &item->security.enabled},
                    {FF_FORMAT_ARG_TYPE_BOOL, &item->security.oneXEnabled},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &item->security.authAlgo},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &item->security.cipherAlgo},
                });
            }

            ffStrbufDestroy(&item->inf.description);
            ffStrbufDestroy(&item->inf.status);
            ffStrbufDestroy(&item->conn.status);
            ffStrbufDestroy(&item->conn.ssid);
            ffStrbufDestroy(&item->conn.macAddress);
            ffStrbufDestroy(&item->conn.phyType);
            ffStrbufDestroy(&item->security.authAlgo);
            ffStrbufDestroy(&item->security.cipherAlgo);
        }
    }
    else
    {
        ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &instance->config.wmTheme, "%s", error);
    }
}
