#pragma once

#include "fastfetch.h"
#include "modules/command/option.h"

void ffPrintCommand(FFinstance* instance, FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
bool ffParseCommandCommandOptions(FFCommandOptions* options, const char* key, const char* value);
void ffDestroyCommandOptions(FFCommandOptions* options);
