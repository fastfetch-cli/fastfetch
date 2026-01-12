#pragma once

#include "option.h"

#define FF_LOGO_MODULE_NAME "Logo"

bool ffPrintLogo(FFLogoOptions* options);
void ffInitLogoOptions(FFLogoOptions* options);
void ffDestroyLogoOptions(FFLogoOptions* options);

extern FFModuleBaseInfo ffLogoModuleInfo;
