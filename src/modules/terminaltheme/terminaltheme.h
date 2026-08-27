#pragma once

#include "option.h"

bool ffPrintTerminalTheme(FFTerminalThemeOptions* options);
void ffInitTerminalThemeOptions(FFTerminalThemeOptions* options);
void ffDestroyTerminalThemeOptions(FFTerminalThemeOptions* options);

extern FFModuleBaseInfo ffTerminalThemeModuleInfo;
