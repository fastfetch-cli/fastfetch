#pragma once

#include "fastfetch.h"

#define FF_SHELL_MODULE_NAME "Shell"

void ffPrintShell(FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value);
void ffDestroyShellOptions(FFShellOptions* options);
void ffParseShellJsonObject(yyjson_val* module);
