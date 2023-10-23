#pragma once

#include "fastfetch.h"

#define FF_TERMINALFONT_MODULE_NAME "TerminalFont"

void ffPrintTerminalFont(FFTerminalFontOptions* options);
void ffInitTerminalFontOptions(FFTerminalFontOptions* options);
void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options);
