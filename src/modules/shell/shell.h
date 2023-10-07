#pragma once

#include "fastfetch.h"

#define FF_SHELL_MODULE_NAME "Shell"

void ffPrintShell(FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value);
void ffDestroyShellOptions(FFShellOptions* options);
void ffParseShellJsonObject(FFShellOptions* options, yyjson_val* module);
void ffGenerateShellJson(FFShellOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintShellHelpFormat(void);
