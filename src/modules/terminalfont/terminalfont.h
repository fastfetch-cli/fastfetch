#pragma once

#include "option.h"

#define FF_TERMINALFONT_MODULE_NAME "TerminalFont"

bool ffPrintTerminalFont(FFTerminalFontOptions* options);
void ffInitTerminalFontOptions(FFTerminalFontOptions* options);
void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options);

extern FFModuleBaseInfo ffTerminalFontModuleInfo;
