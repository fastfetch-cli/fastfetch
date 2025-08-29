#pragma once

#include "option.h"

#define FF_TERMINALTHEME_MODULE_NAME "TerminalTheme"

bool ffPrintTerminalTheme(FFTerminalThemeOptions* options);
void ffInitTerminalThemeOptions(FFTerminalThemeOptions* options);
void ffDestroyTerminalThemeOptions(FFTerminalThemeOptions* options);

extern FFModuleBaseInfo ffTerminalThemeModuleInfo;
