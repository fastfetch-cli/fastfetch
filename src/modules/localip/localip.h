#pragma once

#include "option.h"

bool ffPrintLocalIp(FFLocalIpOptions* options);
void ffInitLocalIpOptions(FFLocalIpOptions* options);
void ffDestroyLocalIpOptions(FFLocalIpOptions* options);

extern FFModuleBaseInfo ffLocalIPModuleInfo;
