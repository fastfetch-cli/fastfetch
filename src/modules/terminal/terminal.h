#pragma once

#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"

void ffPrintTerminal(FFinstance* instance, FFTerminalOptions* options);
void ffInitTerminalOptions(FFTerminalOptions* options);
bool ffParseTerminalCommandOptions(FFTerminalOptions* options, const char* key, const char* value);
void ffDestroyTerminalOptions(FFTerminalOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseTerminalJsonObject(FFinstance* instance, json_object* module);
#endif
