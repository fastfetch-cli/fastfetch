#pragma once

#include "fastfetch.h"

#define FF_LOCALIP_MODULE_NAME "LocalIp"

void ffPrintLocalIp(FFLocalIpOptions* options);
void ffInitLocalIpOptions(FFLocalIpOptions* options);
bool ffParseLocalIpCommandOptions(FFLocalIpOptions* options, const char* key, const char* value);
void ffDestroyLocalIpOptions(FFLocalIpOptions* options);
void ffParseLocalIpJsonObject(yyjson_val* module);
