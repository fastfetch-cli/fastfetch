#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wifi/wifi.h"
#include "modules/wifi/wifi.h"
#include "util/stringUtils.h"

#define FF_WIFI_NUM_FORMAT_ARGS 11

void ffPrintWifi(FFWifiOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFWifiResult));

    const char* error = ffDetectWifi(&result);
    if(error)
    {
        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }
    if(!result.length)
    {
        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No Wifi interfaces found");
        return;
    }

    for(uint32_t index = 0; index < result.length; ++index)
    {
        FFWifiResult* item = (FFWifiResult*)ffListGet(&result, index);
        uint8_t moduleIndex = result.length == 1 ? 0 : (uint8_t)(index + 1);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
            if(item->conn.ssid.length)
            {
                if(item->conn.signalQuality == item->conn.signalQuality)
                {
                    if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                    {
                        ffPercentAppendBar(&buffer, item->conn.signalQuality, options->percent, &options->moduleArgs);
                        ffStrbufAppendC(&buffer, ' ');
                    }
                }

                if (!(instance.config.display.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                {
                    ffStrbufAppend(&buffer, &item->conn.ssid);

                    if(item->conn.protocol.length)
                    {
                        ffStrbufAppendS(&buffer, " - ");
                        ffStrbufAppend(&buffer, &item->conn.protocol);
                    }
                    if(item->conn.security.length)
                    {
                        ffStrbufAppendS(&buffer, " - ");
                        ffStrbufAppend(&buffer, &item->conn.security);
                    }
                    ffStrbufAppendC(&buffer, ' ');
                }

                if(item->conn.signalQuality == item->conn.signalQuality)
                {
                    if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                        ffPercentAppendNum(&buffer, item->conn.signalQuality, options->percent, buffer.length > 0, &options->moduleArgs);
                }

                ffStrbufTrimRight(&buffer, ' ');
            }
            else
            {
                ffStrbufAppend(&buffer, &item->inf.status);
            }
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            FF_STRBUF_AUTO_DESTROY percentNum = ffStrbufCreate();
            ffPercentAppendNum(&percentNum, item->conn.signalQuality, options->percent, false, &options->moduleArgs);
            FF_STRBUF_AUTO_DESTROY percentBar = ffStrbufCreate();
            ffPercentAppendBar(&percentBar, item->conn.signalQuality, options->percent, &options->moduleArgs);
            FF_PRINT_FORMAT_CHECKED(FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_WIFI_NUM_FORMAT_ARGS, ((FFformatarg[]){
                FF_FORMAT_ARG(item->inf.description, "inf-desc"),
                FF_FORMAT_ARG(item->inf.status, "inf-status"),
                FF_FORMAT_ARG(item->conn.status, "status"),
                FF_FORMAT_ARG(item->conn.ssid, "ssid"),
                FF_FORMAT_ARG(item->conn.bssid, "bssid"),
                FF_FORMAT_ARG(item->conn.protocol, "protocol"),
                FF_FORMAT_ARG(percentNum, "signal-quality"),
                FF_FORMAT_ARG(item->conn.rxRate, "rx-rate"),
                FF_FORMAT_ARG(item->conn.txRate, "tx-rate"),
                FF_FORMAT_ARG(item->conn.security, "security"),
                FF_FORMAT_ARG(percentBar, "signal-quality-bar"),
            }));
        }

        ffStrbufDestroy(&item->inf.description);
        ffStrbufDestroy(&item->inf.status);
        ffStrbufDestroy(&item->conn.status);
        ffStrbufDestroy(&item->conn.ssid);
        ffStrbufDestroy(&item->conn.bssid);
        ffStrbufDestroy(&item->conn.protocol);
        ffStrbufDestroy(&item->conn.security);
    }
}

bool ffParseWifiCommandOptions(FFWifiOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WIFI_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseWifiJsonObject(FFWifiOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateWifiJsonConfig(FFWifiOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyWifiOptions))) FFWifiOptions defaultOptions;
    ffInitWifiOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateWifiJsonResult(FF_MAYBE_UNUSED FFWifiOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFWifiResult));
    const char* error = ffDetectWifi(&result);
    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFWifiResult, wifi, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);

        yyjson_mut_val* inf = yyjson_mut_obj_add_obj(doc, obj, "inf");
        yyjson_mut_obj_add_strbuf(doc, inf, "description", &wifi->inf.description);
        yyjson_mut_obj_add_strbuf(doc, inf, "status", &wifi->inf.status);

        yyjson_mut_val* conn = yyjson_mut_obj_add_obj(doc, obj, "conn");
        yyjson_mut_obj_add_strbuf(doc, conn, "status", &wifi->conn.status);
        yyjson_mut_obj_add_strbuf(doc, conn, "ssid", &wifi->conn.ssid);
        yyjson_mut_obj_add_strbuf(doc, conn, "bssid", &wifi->conn.bssid);
        yyjson_mut_obj_add_strbuf(doc, conn, "protocol", &wifi->conn.protocol);
        yyjson_mut_obj_add_strbuf(doc, conn, "security", &wifi->conn.security);
        yyjson_mut_obj_add_real(doc, conn, "signalQuality", wifi->conn.signalQuality);
        yyjson_mut_obj_add_real(doc, conn, "rxRate", wifi->conn.rxRate);
        yyjson_mut_obj_add_real(doc, conn, "txRate", wifi->conn.txRate);
    }

    FF_LIST_FOR_EACH(FFWifiResult, item, result)
    {
        ffStrbufDestroy(&item->inf.description);
        ffStrbufDestroy(&item->inf.status);
        ffStrbufDestroy(&item->conn.status);
        ffStrbufDestroy(&item->conn.ssid);
        ffStrbufDestroy(&item->conn.bssid);
        ffStrbufDestroy(&item->conn.protocol);
        ffStrbufDestroy(&item->conn.security);
    }
}

void ffPrintWifiHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_WIFI_MODULE_NAME, "{4} - {10}", FF_WIFI_NUM_FORMAT_ARGS, ((const char* []) {
        "Interface description - inf-desc",
        "Interface status - inf-status",
        "Connection status - status",
        "Connection SSID - ssid",
        "Connection BSSID - bssid",
        "Connection protocol - protocol",
        "Connection signal quality (percentage num) - signal-quality",
        "Connection RX rate - rx-rate",
        "Connection TX rate - tx-rate",
        "Connection Security algorithm - security",
        "Connection signal quality (percentage bar) - signal-quality-bar",
    }));
}

void ffInitWifiOptions(FFWifiOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_WIFI_MODULE_NAME,
        "Print connected Wi-Fi info (SSID, connection and security protocol)",
        ffParseWifiCommandOptions,
        ffParseWifiJsonObject,
        ffPrintWifi,
        ffGenerateWifiJsonResult,
        ffPrintWifiHelpFormat,
        ffGenerateWifiJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->percent = (FFColorRangeConfig) { 50, 20 };
}

void ffDestroyWifiOptions(FFWifiOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
