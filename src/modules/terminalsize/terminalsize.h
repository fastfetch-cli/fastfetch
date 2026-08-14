#pragma once

#include "option.h"

bool ffPrintTerminalSize(FFTerminalSizeOptions* options);
void ffInitTerminalSizeOptions(FFTerminalSizeOptions* options);
void ffDestroyTerminalSizeOptions(FFTerminalSizeOptions* options);

extern FFModuleBaseInfo ffTerminalSizeModuleInfo;
