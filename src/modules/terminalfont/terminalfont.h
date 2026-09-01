#pragma once

#include "option.h"

bool ffPrintTerminalFont(FFTerminalFontOptions* options);
void ffInitTerminalFontOptions(FFTerminalFontOptions* options);
void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options);

extern FFModuleBaseInfo ffTerminalFontModuleInfo;
