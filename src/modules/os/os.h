#pragma once

#include "fastfetch.h"
#include "modules/os/option.h"

#define FF_OS_MODULE_NAME "OS"

void ffPrintOS(FFinstance* instance, FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value);
void ffDestroyOSOptions(FFOSOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseOSJsonObject(FFinstance* instance, json_object* module);
#endif
