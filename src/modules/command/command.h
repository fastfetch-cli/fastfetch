#pragma once

#include "fastfetch.h"

#define FF_COMMAND_MODULE_NAME "Command"

void ffPrintCommand(FFinstance* instance, FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
bool ffParseCommandCommandOptions(FFCommandOptions* options, const char* key, const char* value);
void ffDestroyCommandOptions(FFCommandOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseCommandJsonObject(FFinstance* instance, json_object* module);
#endif
