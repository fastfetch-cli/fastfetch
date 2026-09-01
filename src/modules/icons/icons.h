#pragma once

#include "option.h"

bool ffPrintIcons(FFIconsOptions* options);
void ffInitIconsOptions(FFIconsOptions* options);
void ffDestroyIconsOptions(FFIconsOptions* options);

extern FFModuleBaseInfo ffIconsModuleInfo;
