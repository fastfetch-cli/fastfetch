#pragma once

#include "fastfetch.h"

#define FF_LOCALIP_MODULE_NAME "LocalIp"

void ffPrintLocalIp(FFinstance* instance, FFLocalIpOptions* options);
void ffInitLocalIpOptions(FFLocalIpOptions* options);
bool ffParseLocalIpCommandOptions(FFLocalIpOptions* options, const char* key, const char* value);
void ffDestroyLocalIpOptions(FFLocalIpOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseLocalIpJsonObject(FFinstance* instance, json_object* module);
#endif
