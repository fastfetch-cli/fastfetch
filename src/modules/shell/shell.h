#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_SHELL_MODULE_NAME "Shell"

void ffPrintShell(FFinstance* instance, FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value);
void ffDestroyShellOptions(FFShellOptions* options);
void ffParseShellJsonObject(FFinstance* instance, yyjson_val* module);
