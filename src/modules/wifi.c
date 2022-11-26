#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wifi/wifi.h"

#define FF_WIFI_MODULE_NAME "Wifi"
#define FF_WIFI_NUM_FORMAT_ARGS 13

void ffPrintWifi(FFinstance* instance)
{
    FFWifiResult result;
    ffStrbufInit(&result.inf.description);
    ffStrbufInit(&result.inf.status);
    ffStrbufInit(&result.conn.status);
    ffStrbufInit(&result.conn.ssid);
    ffStrbufInit(&result.conn.macAddress);
    ffStrbufInit(&result.conn.phyType);
    result.conn.signalQuality = 0.0/0.0;
    result.conn.rxRate = 0.0/0.0;
    result.conn.txRate = 0.0/0.0;
    result.security.enabled = false;
    result.security.oneXEnabled = false;
    ffStrbufInit(&result.security.authAlgo);
    ffStrbufInit(&result.security.cipherAlgo);
    ffStrbufInit(&result.error);

    ffDetectWifi(instance, &result);

    if(!result.error.length)
    {
        if(instance->config.wifi.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_WIFI_MODULE_NAME, 0, &instance->config.wifi.key);
            if(result.conn.ssid.length)
            {
                printf("%s - %s", result.conn.ssid.chars, result.conn.phyType.chars);
                if(!result.security.enabled)
                    puts(" - insecure");
                else
                    putchar('\n');
            }
            else
            {
                puts(result.inf.status.chars);
            }
        }
        else
        {
            ffPrintFormat(instance, FF_WIFI_MODULE_NAME, 0, &instance->config.wifi, FF_WIFI_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.inf.description},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.inf.status},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.conn.status},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.conn.ssid},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.conn.macAddress},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.conn.phyType},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &result.conn.signalQuality},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &result.conn.rxRate},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &result.conn.txRate},
                {FF_FORMAT_ARG_TYPE_BOOL, &result.security.enabled},
                {FF_FORMAT_ARG_TYPE_BOOL, &result.security.oneXEnabled},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.security.authAlgo},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.security.cipherAlgo},
            });
        }
    }
    else
    {
        ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &instance->config.wmTheme, "%*s", result.error.length, result.error.chars);
    }

    ffStrbufDestroy(&result.inf.description);
    ffStrbufDestroy(&result.inf.status);
    ffStrbufDestroy(&result.conn.status);
    ffStrbufDestroy(&result.conn.ssid);
    ffStrbufDestroy(&result.conn.macAddress);
    ffStrbufDestroy(&result.conn.phyType);
    ffStrbufDestroy(&result.security.authAlgo);
    ffStrbufDestroy(&result.security.cipherAlgo);
    ffStrbufDestroy(&result.error);
}
