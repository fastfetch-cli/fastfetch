#pragma once

#include "fastfetch.h"

#define FF_WIFI_MODULE_NAME "Wifi"

void ffPrintWifi(FFWifiOptions* options);
void ffInitWifiOptions(FFWifiOptions* options);
void ffDestroyWifiOptions(FFWifiOptions* options);
