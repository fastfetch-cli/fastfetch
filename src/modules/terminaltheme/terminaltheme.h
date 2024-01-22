#pragma once

#include "fastfetch.h"

#define FF_TERMINALTHEME_MODULE_NAME "TerminalTheme"

void ffPrintTerminalTheme(FFTerminalThemeOptions* options);
void ffInitTerminalThemeOptions(FFTerminalThemeOptions* options);
void ffDestroyTerminalThemeOptions(FFTerminalThemeOptions* options);
