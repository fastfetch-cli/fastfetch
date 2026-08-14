#pragma once

#include "option.h"

bool ffPrintTerminal(FFTerminalOptions* options);
void ffInitTerminalOptions(FFTerminalOptions* options);
void ffDestroyTerminalOptions(FFTerminalOptions* options);

extern FFModuleBaseInfo ffTerminalModuleInfo;
