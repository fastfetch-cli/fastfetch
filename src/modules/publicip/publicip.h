#pragma once

#include "fastfetch.h"

#define FF_PUBLICIP_MODULE_NAME "PublicIp"

void ffPreparePublicIp(FFPublicIpOptions* options);

void ffPrintPublicIp(FFinstance* instance, FFPublicIpOptions* options);
void ffInitPublicIpOptions(FFPublicIpOptions* options);
bool ffParsePublicIpCommandOptions(FFPublicIpOptions* options, const char* key, const char* value);
void ffDestroyPublicIpOptions(FFPublicIpOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParsePublicIpJsonObject(FFinstance* instance, json_object* module);
#endif
