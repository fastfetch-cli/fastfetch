#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_TERMINALFONT_MODULE_NAME "TerminalFont"

void ffPrintTerminalFont(FFinstance* instance, FFTerminalFontOptions* options);
void ffInitTerminalFontOptions(FFTerminalFontOptions* options);
bool ffParseTerminalFontCommandOptions(FFTerminalFontOptions* options, const char* key, const char* value);
void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options);
void ffParseTerminalFontJsonObject(FFinstance* instance, yyjson_val* module);
