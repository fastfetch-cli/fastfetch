#pragma once

#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"

void ffPrintTerminal(FFTerminalOptions* options);
void ffInitTerminalOptions(FFTerminalOptions* options);
bool ffParseTerminalCommandOptions(FFTerminalOptions* options, const char* key, const char* value);
void ffDestroyTerminalOptions(FFTerminalOptions* options);
void ffParseTerminalJsonObject(FFTerminalOptions* options, yyjson_val* module);
void ffGenerateTerminalJson(FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
