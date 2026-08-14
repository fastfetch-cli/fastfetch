#pragma once

#include "option.h"

void ffPreparePublicIp(FFPublicIPOptions* options);

bool ffPrintPublicIp(FFPublicIPOptions* options);
void ffInitPublicIpOptions(FFPublicIPOptions* options);
void ffDestroyPublicIpOptions(FFPublicIPOptions* options);

extern FFModuleBaseInfo ffPublicIPModuleInfo;
