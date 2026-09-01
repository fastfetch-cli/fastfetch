#pragma once

#include "option.h"

bool ffPrintDNS(FFDNSOptions* options);
void ffInitDNSOptions(FFDNSOptions* options);
void ffDestroyDNSOptions(FFDNSOptions* options);

extern FFModuleBaseInfo ffDNSModuleInfo;
