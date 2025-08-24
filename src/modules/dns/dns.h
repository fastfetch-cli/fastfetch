#pragma once

#include "option.h"

#define FF_DNS_MODULE_NAME "DNS"

bool ffPrintDNS(FFDNSOptions* options);
void ffInitDNSOptions(FFDNSOptions* options);
void ffDestroyDNSOptions(FFDNSOptions* options);

extern FFModuleBaseInfo ffDNSModuleInfo;
