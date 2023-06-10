#pragma once

#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"

void ffPrintTerminal(FFinstance* instance, FFTerminalOptions* options);
void ffInitTerminalOptions(FFTerminalOptions* options);
bool ffParseTerminalCommandOptions(FFTerminalOptions* options, const char* key, const char* value);
void ffDestroyTerminalOptions(FFTerminalOptions* options);
void ffParseTerminalJsonObject(FFinstance* instance, yyjson_val* module);
