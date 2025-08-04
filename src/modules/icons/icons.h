#pragma once

#include "option.h"

#define FF_ICONS_MODULE_NAME "Icons"

void ffPrintIcons(FFIconsOptions* options);
void ffInitIconsOptions(FFIconsOptions* options);
void ffDestroyIconsOptions(FFIconsOptions* options);

extern FFModuleBaseInfo ffIconsModuleInfo;
