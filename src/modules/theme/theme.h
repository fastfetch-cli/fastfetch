#pragma once

#include "option.h"

bool ffPrintTheme(FFThemeOptions* options);
void ffInitThemeOptions(FFThemeOptions* options);
void ffDestroyThemeOptions(FFThemeOptions* options);

extern FFModuleBaseInfo ffThemeModuleInfo;
