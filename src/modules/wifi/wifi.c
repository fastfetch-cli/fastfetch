#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wifi/wifi.h"
#include "modules/wifi/wifi.h"
#include "util/stringUtils.h"

#define FF_WIFI_NUM_FORMAT_ARGS 10

void ffPrintWifi(FFWifiOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFWifiResult));

    const char* error = ffDetectWifi(&result);
    if(error)
    {
        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }
    if(!result.length)
    {
        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, "No Wifi interfaces found");
        return;
    }

    for(uint32_t index = 0; index < result.length; ++index)
    {
        FFWifiResult* item = (FFWifiResult*)ffListGet(&result, index);
        uint8_t moduleIndex = result.length == 1 ? 0 : (uint8_t)(index + 1);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
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
            ffPrintFormat(FF_WIFI_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_WIFI_NUM_FORMAT_ARGS, (FFformatarg[]){
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

bool ffParseWifiCommandOptions(FFWifiOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WIFI_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
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

        ffPrintError(FF_WIFI_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateWifiJsonConfig(FFWifiOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyWifiOptions))) FFWifiOptions defaultOptions;
    ffInitWifiOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
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
    if(!result.length)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No Wifi interfaces found");
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
        yyjson_mut_obj_add_strbuf(doc, conn, "macAddress", &wifi->conn.macAddress);
        yyjson_mut_obj_add_strbuf(doc, conn, "protocol", &wifi->conn.protocol);
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
        ffStrbufDestroy(&item->conn.macAddress);
        ffStrbufDestroy(&item->conn.protocol);
        ffStrbufDestroy(&item->conn.security);
    }
}

void ffPrintWifiHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_WIFI_MODULE_NAME, "{4} - {10}", FF_WIFI_NUM_FORMAT_ARGS, (const char* []) {
        "Interface description",
        "Interface status",
        "Connection status",
        "Connection SSID",
        "Connection mac address",
        "Connection protocol",
        "Connection signal quality (percentage)",
        "Connection RX rate",
        "Connection TX rate",
        "Connection Security algorithm"
    });
}

void ffInitWifiOptions(FFWifiOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_WIFI_MODULE_NAME,
        ffParseWifiCommandOptions,
        ffParseWifiJsonObject,
        ffPrintWifi,
        ffGenerateWifiJsonResult,
        ffPrintWifiHelpFormat,
        ffGenerateWifiJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyWifiOptions(FFWifiOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
