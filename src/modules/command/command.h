#pragma once

#include "fastfetch.h"
#include "modules/command/option.h"

void ffPrintCommand(FFinstance* instance, FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
bool ffParseCommandCommandOptions(FFCommandOptions* options, const char* key, const char* value);
void ffDestroyCommandOptions(FFCommandOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseCommandJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif
