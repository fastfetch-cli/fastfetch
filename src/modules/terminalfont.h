#pragma once

#ifndef FF_INCLUDED_modules_terminalfont
#define FF_INCLUDED_modules_terminalfont

#include "fastfetch.h"
#include "detection/terminalshell.h"

#define FF_TERMFONT_MODULE_NAME "Terminal Font"
#define FF_TERMFONT_NUM_FORMAT_ARGS 5

void ffPrintTerminalFontResult(FFinstance* instance, const char* raw, FFfont* font);
bool ffPrintTerminalFontPlatform(FFinstance* instance, const FFTerminalShellResult* shellInfo);

#endif
