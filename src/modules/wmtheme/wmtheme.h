#pragma once

#include "option.h"

bool ffPrintWMTheme(FFWMThemeOptions* options);
void ffInitWMThemeOptions(FFWMThemeOptions* options);
void ffDestroyWMThemeOptions(FFWMThemeOptions* options);

extern FFModuleBaseInfo ffWMThemeModuleInfo;
