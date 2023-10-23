#pragma once

#include "fastfetch.h"

#define FF_LOCALIP_MODULE_NAME "LocalIp"

void ffPrintLocalIp(FFLocalIpOptions* options);
void ffInitLocalIpOptions(FFLocalIpOptions* options);
void ffDestroyLocalIpOptions(FFLocalIpOptions* options);
