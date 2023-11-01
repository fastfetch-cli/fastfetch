#pragma once

#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"

void ffPrintTerminal(FFTerminalOptions* options);
void ffInitTerminalOptions(FFTerminalOptions* options);
void ffDestroyTerminalOptions(FFTerminalOptions* options);
