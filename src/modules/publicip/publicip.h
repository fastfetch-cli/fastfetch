#pragma once

#include "option.h"

#define FF_PUBLICIP_MODULE_NAME "PublicIp"

void ffPreparePublicIp(FFPublicIPOptions* options);

bool ffPrintPublicIp(FFPublicIPOptions* options);
void ffInitPublicIpOptions(FFPublicIPOptions* options);
void ffDestroyPublicIpOptions(FFPublicIPOptions* options);

extern FFModuleBaseInfo ffPublicIPModuleInfo;
