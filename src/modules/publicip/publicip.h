#pragma once

#include "fastfetch.h"

#define FF_PUBLICIP_MODULE_NAME "PublicIp"

void ffPreparePublicIp(FFPublicIpOptions* options);

void ffPrintPublicIp(FFPublicIpOptions* options);
void ffInitPublicIpOptions(FFPublicIpOptions* options);
bool ffParsePublicIpCommandOptions(FFPublicIpOptions* options, const char* key, const char* value);
void ffDestroyPublicIpOptions(FFPublicIpOptions* options);
void ffParsePublicIpJsonObject(FFPublicIpOptions* options, yyjson_val* module);
