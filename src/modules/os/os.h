#pragma once

#include "fastfetch.h"
#include "modules/os/option.h"

void ffPrintOS(FFinstance* instance, FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value);
void ffDestroyOSOptions(FFOSOptions* options);
