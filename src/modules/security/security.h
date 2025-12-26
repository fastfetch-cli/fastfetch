#pragma once

#include "option.h"

#define FF_SECURITY_MODULE_NAME "Security"

bool ffPrintSecurity(FFSecurityOptions* options);
void ffInitSecurityOptions(FFSecurityOptions* options);
void ffDestroySecurityOptions(FFSecurityOptions* options);

extern FFModuleBaseInfo ffSecurityModuleInfo;
