#pragma once

#include "fastfetch.h"
#include "modules/wifi/option.h"

#define FF_WIFI_MODULE_NAME "Wifi"

void ffPrintWifi(FFinstance* instance, FFWifiOptions* options);
void ffInitWifiOptions(FFWifiOptions* options);
bool ffParseWifiCommandOptions(FFWifiOptions* options, const char* key, const char* value);
void ffDestroyWifiOptions(FFWifiOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseWifiJsonObject(FFinstance* instance, json_object* module);
#endif
