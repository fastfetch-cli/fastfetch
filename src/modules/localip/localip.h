#pragma once

#include "fastfetch.h"

#define FF_LOCALIP_MODULE_NAME "LocalIp"

void ffPrintLocalIp(FFLocalIpOptions* options);
void ffInitLocalIpOptions(FFLocalIpOptions* options);
bool ffParseLocalIpCommandOptions(FFLocalIpOptions* options, const char* key, const char* value);
void ffDestroyLocalIpOptions(FFLocalIpOptions* options);
void ffParseLocalIpJsonObject(FFLocalIpOptions* options, yyjson_val* module);
void ffGenerateLocalIpJson(FFLocalIpOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintLocalIpHelpFormat(void);
