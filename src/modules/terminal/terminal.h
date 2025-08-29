#pragma once

#include "option.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"

bool ffPrintTerminal(FFTerminalOptions* options);
void ffInitTerminalOptions(FFTerminalOptions* options);
void ffDestroyTerminalOptions(FFTerminalOptions* options);

extern FFModuleBaseInfo ffTerminalModuleInfo;
