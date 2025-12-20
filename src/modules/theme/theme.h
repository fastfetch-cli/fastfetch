#pragma once

#include "option.h"

#define FF_THEME_MODULE_NAME "Theme"

bool ffPrintTheme(FFThemeOptions* options);
void ffInitThemeOptions(FFThemeOptions* options);
void ffDestroyThemeOptions(FFThemeOptions* options);

extern FFModuleBaseInfo ffThemeModuleInfo;
