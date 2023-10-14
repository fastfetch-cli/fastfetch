#pragma once

#include "fastfetch.h"

#define FF_WIFI_MODULE_NAME "Wifi"

void ffPrintWifi(FFWifiOptions* options);
void ffInitWifiOptions(FFWifiOptions* options);
bool ffParseWifiCommandOptions(FFWifiOptions* options, const char* key, const char* value);
void ffDestroyWifiOptions(FFWifiOptions* options);
void ffParseWifiJsonObject(FFWifiOptions* options, yyjson_val* module);
void ffGenerateWifiJson(FFWifiOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintWifiHelpFormat(void);
