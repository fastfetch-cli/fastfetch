#pragma once

#include "fastfetch.h"

#define FF_TERMINALFONT_MODULE_NAME "TerminalFont"

void ffPrintTerminalFont(FFinstance* instance, FFTerminalFontOptions* options);
void ffInitTerminalFontOptions(FFTerminalFontOptions* options);
bool ffParseTerminalFontCommandOptions(FFTerminalFontOptions* options, const char* key, const char* value);
void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseTerminalFontJsonObject(FFinstance* instance, json_object* module);
#endif
