#pragma once

#include "fastfetch.h"

#define FF_SHELL_MODULE_NAME "Shell"

void ffPrintShell(FFinstance* instance, FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value);
void ffDestroyShellOptions(FFShellOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseShellJsonObject(FFinstance* instance, json_object* module);
#endif
