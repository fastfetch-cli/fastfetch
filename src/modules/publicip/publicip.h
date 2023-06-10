#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_PUBLICIP_MODULE_NAME "PublicIp"

void ffPreparePublicIp(FFPublicIpOptions* options);

void ffPrintPublicIp(FFinstance* instance, FFPublicIpOptions* options);
void ffInitPublicIpOptions(FFPublicIpOptions* options);
bool ffParsePublicIpCommandOptions(FFPublicIpOptions* options, const char* key, const char* value);
void ffDestroyPublicIpOptions(FFPublicIpOptions* options);
void ffParsePublicIpJsonObject(FFinstance* instance, yyjson_val* module);
