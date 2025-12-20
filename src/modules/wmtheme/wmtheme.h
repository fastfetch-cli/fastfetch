#pragma once

#include "option.h"

#define FF_WMTHEME_MODULE_NAME "WMTheme"

bool ffPrintWMTheme(FFWMThemeOptions* options);
void ffInitWMThemeOptions(FFWMThemeOptions* options);
void ffDestroyWMThemeOptions(FFWMThemeOptions* options);

extern FFModuleBaseInfo ffWMThemeModuleInfo;
