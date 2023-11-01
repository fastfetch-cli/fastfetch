#pragma once

#include "fastfetch.h"

#define FF_TERMINALSIZE_MODULE_NAME "TerminalSize"

void ffPrintTerminalSize(FFTerminalSizeOptions* options);
void ffInitTerminalSizeOptions(FFTerminalSizeOptions* options);
void ffDestroyTerminalSizeOptions(FFTerminalSizeOptions* options);
