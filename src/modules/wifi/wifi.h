#pragma once

#include "option.h"

#define FF_WIFI_MODULE_NAME "Wifi"

bool ffPrintWifi(FFWifiOptions* options);
void ffInitWifiOptions(FFWifiOptions* options);
void ffDestroyWifiOptions(FFWifiOptions* options);

extern FFModuleBaseInfo ffWifiModuleInfo;
