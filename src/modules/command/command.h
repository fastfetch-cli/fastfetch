#pragma once

#include "fastfetch.h"

void ffPrintCommand(FFinstance* instance, FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
bool ffParseCommandCommandOptions(FFCommandOptions* options, const char* key, const char* value);
void ffDestroyCommandOptions(FFCommandOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
bool ffParseCommandJsonObject(FFinstance* instance, const char* type, json_object* module);
#endif
