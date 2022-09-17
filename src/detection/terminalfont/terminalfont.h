#pragma once

#ifndef FF_INCLUDED_detection_terminalfont
#define FF_INCLUDED_detection_terminalfont

#include "fastfetch.h"
#include "detection/terminalshell.h"

const char* ffDetectTerminalFontPlatform(FFinstance* instance, const FFTerminalShellResult* shellInfo, FFfont* font);
const char* ffDetectTerminalFontCommon(FFinstance* instance, FFfont* font);

#endif
