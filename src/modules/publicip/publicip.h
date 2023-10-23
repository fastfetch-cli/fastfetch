#pragma once

#include "fastfetch.h"

#define FF_PUBLICIP_MODULE_NAME "PublicIp"

void ffPreparePublicIp(FFPublicIpOptions* options);

void ffPrintPublicIp(FFPublicIpOptions* options);
void ffInitPublicIpOptions(FFPublicIpOptions* options);
void ffDestroyPublicIpOptions(FFPublicIpOptions* options);
