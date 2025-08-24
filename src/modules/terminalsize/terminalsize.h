#pragma once

#include "option.h"

#define FF_TERMINALSIZE_MODULE_NAME "TerminalSize"

bool ffPrintTerminalSize(FFTerminalSizeOptions* options);
void ffInitTerminalSizeOptions(FFTerminalSizeOptions* options);
void ffDestroyTerminalSizeOptions(FFTerminalSizeOptions* options);

extern FFModuleBaseInfo ffTerminalSizeModuleInfo;
