#pragma once

#include "option.h"

bool ffPrintLogo(FFLogoOptions* options);
void ffInitLogoOptions(FFLogoOptions* options);
void ffDestroyLogoOptions(FFLogoOptions* options);

extern FFModuleBaseInfo ffLogoModuleInfo;
