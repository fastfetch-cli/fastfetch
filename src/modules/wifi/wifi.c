#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wifi/wifi.h"
#include "modules/wifi/wifi.h"
#include "util/stringUtils.h"

bool ffPrintWifi(FFWifiOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFWifiResult));

    const char* error = ffDetectWifi(&result);
    if(error)
    {
        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }
    if(!result.length)
    {
        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No Wifi interfaces found");
        return false;
    }

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    for(uint32_t index = 0; index < result.length; ++index)
    {
        FFWifiResult* item = FF_LIST_GET(FFWifiResult, result, index);
        uint8_t moduleIndex = result.length == 1 ? 0 : (uint8_t)(index + 1);

        // https://en.wikipedia.org/wiki/List_of_WLAN_channels
        char bandStr[8];
        if (item->conn.frequency > 58000)
            strcpy(bandStr, "60");
        if (item->conn.frequency > 40000)
            strcpy(bandStr, "45");
        else if (item->conn.frequency > 5900)
            strcpy(bandStr, "6");
        else if (item->conn.frequency > 5100)
            strcpy(bandStr, "5");
        else if (item->conn.frequency > 4900)
            strcpy(bandStr, "4.9");
        else if (item->conn.frequency > 3600)
            strcpy(bandStr, "3.65");
        else if (item->conn.frequency > 2000)
            strcpy(bandStr, "2.4");
        else if (item->conn.frequency > 800)
            strcpy(bandStr, "0.9");
        else
            bandStr[0] = '\0';

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
            if(item->conn.ssid.length)
            {
                if(item->conn.signalQuality != -DBL_MAX)
                {
                    if(percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                    {
                        ffPercentAppendBar(&buffer, item->conn.signalQuality, options->percent, &options->moduleArgs);
                        ffStrbufAppendC(&buffer, ' ');
                    }
                }

                if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                {
                    ffStrbufAppend(&buffer, &item->conn.ssid);

                    if(item->conn.protocol.length)
                    {
                        ffStrbufAppendS(&buffer, " - ");
                        ffStrbufAppend(&buffer, &item->conn.protocol);
                    }
                    if (bandStr[0])
                    {
                        ffStrbufAppendF(&buffer, " - %s%sGHz", bandStr,
                            instance.config.display.freqSpaceBeforeUnit == FF_SPACE_BEFORE_UNIT_NEVER ? "" : " ");
                    }
                    if(item->conn.security.length)
                    {
                        ffStrbufAppendS(&buffer, " - ");
                        ffStrbufAppend(&buffer, &item->conn.security);
                    }
                    ffStrbufAppendC(&buffer, ' ');
                }

                if(item->conn.signalQuality != -DBL_MAX)
                {
                    if(percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
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
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffPercentAppendNum(&percentNum, item->conn.signalQuality, options->percent, false, &options->moduleArgs);
            FF_STRBUF_AUTO_DESTROY percentBar = ffStrbufCreate();
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&percentBar, item->conn.signalQuality, options->percent, &options->moduleArgs);

            FF_PRINT_FORMAT_CHECKED(FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
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
                FF_FORMAT_ARG(item->conn.channel, "channel"),
                FF_FORMAT_ARG(bandStr, "band"),
            }));
        }
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

    return true;
}

void ffParseWifiJsonObject(FFWifiOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateWifiJsonConfig(FFWifiOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateWifiJsonResult(FF_MAYBE_UNUSED FFWifiOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFWifiResult));
    const char* error = ffDetectWifi(&result);
    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
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
        if (wifi->conn.signalQuality != -DBL_MAX)
            yyjson_mut_obj_add_real(doc, conn, "signalQuality", wifi->conn.signalQuality);
        else
            yyjson_mut_obj_add_null(doc, conn, "signalQuality");
        if (wifi->conn.rxRate != -DBL_MAX)
            yyjson_mut_obj_add_real(doc, conn, "rxRate", wifi->conn.rxRate);
        else
            yyjson_mut_obj_add_null(doc, conn, "rxRate");
        if (wifi->conn.txRate != -DBL_MAX)
            yyjson_mut_obj_add_real(doc, conn, "txRate", wifi->conn.txRate);
        else
            yyjson_mut_obj_add_null(doc, conn, "txRate");
        yyjson_mut_obj_add_uint(doc, conn, "channel", wifi->conn.channel);
        yyjson_mut_obj_add_uint(doc, conn, "frequency", wifi->conn.frequency);
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

    return true;
}

void ffInitWifiOptions(FFWifiOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->percent = (FFPercentageModuleConfig) { 75, 50, 0 };
}

void ffDestroyWifiOptions(FFWifiOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffWifiModuleInfo = {
    .name = FF_WIFI_MODULE_NAME,
    .description = "Print connected Wi-Fi info (SSID, connection and security protocol)",
    .initOptions = (void*) ffInitWifiOptions,
    .destroyOptions = (void*) ffDestroyWifiOptions,
    .parseJsonObject = (void*) ffParseWifiJsonObject,
    .printModule = (void*) ffPrintWifi,
    .generateJsonResult = (void*) ffGenerateWifiJsonResult,
    .generateJsonConfig = (void*) ffGenerateWifiJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Interface description", "inf-desc"},
        {"Interface status", "inf-status"},
        {"Connection status", "status"},
        {"Connection SSID", "ssid"},
        {"Connection BSSID", "bssid"},
        {"Connection protocol", "protocol"},
        {"Connection signal quality (percentage num)", "signal-quality"},
        {"Connection RX rate", "rx-rate"},
        {"Connection TX rate", "tx-rate"},
        {"Connection Security algorithm", "security"},
        {"Connection signal quality (percentage bar)", "signal-quality-bar"},
        {"Connection channel number", "channel"},
        {"Connection channel band in GHz", "band"},
    }))
};
