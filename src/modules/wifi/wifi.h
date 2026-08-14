#pragma once

#include "option.h"

bool ffPrintWifi(FFWifiOptions* options);
void ffInitWifiOptions(FFWifiOptions* options);
void ffDestroyWifiOptions(FFWifiOptions* options);

extern FFModuleBaseInfo ffWifiModuleInfo;
