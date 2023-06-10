#pragma once

#include "fastfetch.h"

#define FF_WIFI_MODULE_NAME "Wifi"

void ffPrintWifi(FFinstance* instance, FFWifiOptions* options);
void ffInitWifiOptions(FFWifiOptions* options);
bool ffParseWifiCommandOptions(FFWifiOptions* options, const char* key, const char* value);
void ffDestroyWifiOptions(FFWifiOptions* options);
void ffParseWifiJsonObject(FFinstance* instance, yyjson_val* module);
