#pragma once

#include "option.h"

#define FF_LOCALIP_MODULE_NAME "LocalIp"

bool ffPrintLocalIp(FFLocalIpOptions* options);
void ffInitLocalIpOptions(FFLocalIpOptions* options);
void ffDestroyLocalIpOptions(FFLocalIpOptions* options);

extern FFModuleBaseInfo ffLocalIPModuleInfo;
