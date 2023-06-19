#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wifi/wifi.h"
#include "modules/wifi/wifi.h"
#include "util/stringUtils.h"

#define FF_WIFI_NUM_FORMAT_ARGS 10

void ffPrintWifi(FFinstance* instance, FFWifiOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFWifiResult));

    const char* error = ffDetectWifi(instance, &result);
    if(error)
    {
        ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }
    if(!result.length)
    {
        ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, "No Wifi interfaces found");
        return;
    }

    for(uint32_t index = 0; index < result.length; ++index)
    {
        FFWifiResult* item = (FFWifiResult*)ffListGet(&result, index);
        uint8_t moduleIndex = result.length == 1 ? 0 : (uint8_t)(index + 1);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs.key, &options->moduleArgs.keyColor);
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
            ffPrintFormat(instance, FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_WIFI_NUM_FORMAT_ARGS, (FFformatarg[]){
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

void ffInitWifiOptions(FFWifiOptions* options)
{
    options->moduleName = FF_WIFI_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseWifiCommandOptions(FFWifiOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WIFI_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyWifiOptions(FFWifiOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseWifiJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFWifiOptions __attribute__((__cleanup__(ffDestroyWifiOptions))) options;
    ffInitWifiOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_WIFI_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWifi(instance, &options);
}
