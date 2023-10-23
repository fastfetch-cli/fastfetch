#pragma once

#include "fastfetch.h"

#define FF_TERMINALFONT_MODULE_NAME "TerminalFont"

void ffPrintTerminalFont(FFTerminalFontOptions* options);
void ffInitTerminalFontOptions(FFTerminalFontOptions* options);
bool ffParseTerminalFontCommandOptions(FFTerminalFontOptions* options, const char* key, const char* value);
void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options);
void ffParseTerminalFontJsonObject(FFTerminalFontOptions* options, yyjson_val* module);
void ffGenerateTerminalFontJsonResult(FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintTerminalFontHelpFormat(void);
